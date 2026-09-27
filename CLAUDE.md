# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A CPU ray tracer in C++ (originally a university thesis with a Qt GUI), sped up with multithreading and SSE. The original thesis version lives on the `thesis-release` branch. `master` is continuing it as an experiment: building an MCP (Model Context Protocol) server so AI assistants can set up scenes and render ray-traced images. The ray tracing core is Qt-free, and a command line renderer (`raytracer-cli`) renders scene files without Qt.

## Build

It's a CMake project ([CMakeLists.txt](CMakeLists.txt)). There are no tests, lint config or CI.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release    # MSYS2 MinGW: add -G "MinGW Makefiles"
cmake --build build/release
```

- Targets:
  - `raytracer_core`: static library with `math/`, `objects/`, `raytracer/`, `scene/` and tinyobjloader. **It must stay Qt-free.** It doesn't link Qt, so a Qt include there fails to compile.
  - `raytracer-cli` ([cli/main.cpp](cli/main.cpp)): links only the core. MinGW links it with `-static`, so it needs no MinGW DLLs. After each build, `resources/obj` is copied next to it.
  - `raytracer`: the GUI. It's built only when **Qt 5** is found (Core, Gui, Widgets, OpenGL); `-DRAYTRACER_BUILD_GUI=OFF` skips it. The viewport uses `QGLWidget`, which was removed in Qt 6.
- C++11, x86-64. GCC/Clang get `-msse -msse4.1`; the code only uses SSE/SSE2 intrinsics, so MSVC needs no flag. Verified with MSYS2 MinGW and MSVC 2022 (CLI only, no Qt for MSVC here) on Windows and GCC on Linux; all three render identical pixels.
- [.vscode/tasks.json](.vscode/tasks.json) builds `build/debug` and `build/release` with MSYS2 MinGW (`C:\msys64\mingw64\bin`).
- The build only compiles files listed in `CMakeLists.txt`. When you add a file, add it there too. Some files in the repo are **not** built: `cpu_info_window.*` and `raytracer/rt_packed_data.cpp` (an old standalone SSE scratch test that includes `stdafx.h`).
- Meshes and shaders: [resources/resources.qrc](resources/resources.qrc) embeds the `.obj` meshes and GLSL shaders into the GUI (`:/obj/...`, `:/shaders/...`). The subfolders mirror the resource paths: `resources/obj/sphere.obj` is `:/obj/sphere.obj`. The core loads meshes from files, not Qt resources. The GUI copies `:/obj` into a temp folder at startup, and the CLI reads the `resources/obj` folder next to its executable (or `--resources <dir>`).

## Command line

```sh
raytracer-cli scenes/example.sc -o example.png [--width 800] [--height 600] [--threads N] [--depth 3] [--scalar] [--resources <dir>]
```

The defaults match the GUI: 800×600 and trace depth 3. It uses all hardware threads, and SSE unless `--scalar` is given (`--scalar` is the GUI's plain Render button). Exit codes: 1 usage error, 2 scene or mesh error, 3 the image can't be written.

## Architecture

**Two parallel representations of each object.** `RTObject` ([objects/rt_object.h](objects/rt_object.h)) holds the ray tracing data: the triangle vertices and the face and vertex normals, all in object space. It also holds the world-space triangles that `intersect` tests: the first vertex and two edges of each triangle, stored both plain and SSE-packed. `updateWorldSpace()` rebuilds them from the position and scale. `RTRenderer::render` calls it for every object, and GUI picking calls it before each test, because the GUI moves objects without rebuilding them. Code that calls `intersect` or `RTTracer::castRay` directly must call it after any position or scale change; note that `RTScene::openScene` sets the scale after loading. For the GUI it also carries a `GLObject*` ([opengl/gl_object.h](opengl/gl_object.h)), which the core only forward-declares. `GLWidget::paintGL` creates the `GLObject` from `RTObject::getVertices()` the first time it draws the object, while the GL context is current. Lights (`RTLight` → `RTPointLight`, `RTDistantLight`) follow the same pattern. Every primitive (sphere, cube, torus, …) is a triangle mesh that tinyobjloader loads from the scene's mesh folder (`RTScene::setMeshDirectory`); only the first shape is used. There are no analytic primitives.

**Rendering flow** ([raytracer/rt_renderer.cpp](raytracer/rt_renderer.cpp)), shared by the GUI (`MainWindow::renderImage`) and the CLI:
1. `RTRenderer::render` copies the camera and sizes it to the image.
2. It updates every object's world-space triangles and builds an `RTBVH` over them (a few ms; `getRenderTime()` includes it).
3. It splits the image into tiles, clipped at the image edges, in `RTImageParts`, a mutex-guarded work queue.
4. It runs N `RTTracer`s on `std::thread`s. Each one takes tiles until none are left, and `render` blocks until all threads finish.
5. It returns an `RTImage` ([raytracer/rt_image.h](raytracer/rt_image.h)): float RGB pixels, `toRGB8()` (clamp to [0, 1], ×255, truncate), and `savePNG()` via `3rd_party/stb_image_write.h`. The GUI shows it in `ImageViewer` and saves `savedFromProg_<timestamp>.png` in the current directory.

**Tracer** ([raytracer/rt_tracer.cpp](raytracer/rt_tracer.cpp)): `castRay` finds each hit with `Trace`, through the BVH that `RTRenderer` passes with `setBVH`. Without one, for example when `castRay` is called directly, it tests every object, which means every triangle. It shades based on `RTMaterialType` (Diffuse, Specular, DiffuseAndSpecular, Reflective), casts hard shadow rays, and recurses for reflection up to `MaxTraceDepth`. Shadow rays go through `isOccluded`: with a BVH, `RTBVH::occluded` stops at the first triangle it finds (any hit); without one, it uses `Trace`. Only triangles closer than the light count: `RTLight::illuminate` returns the light's distance, and the largest float for a distant light. Smooth shading interpolates vertex normals with the barycentric `u, v` from the triangle hit.

**BVH** ([raytracer/rt_bvh.h](raytracer/rt_bvh.h)): a 4-wide tree (BVH4) over the world-space triangles of all objects. It's built as a binary tree with a binned surface area heuristic, then collapsed so each node has up to 4 children. Their boxes are stored side by side, one SSE register per bound, and missing children have inverted boxes. Each leaf has 4 triangle slots in one SSE pack, and unused slots are degenerate. It uses the same triangle tests as `RTObject::intersect`, in [raytracer/rt_triangle.h](raytracer/rt_triangle.h), and breaks ties at equal distance by the lowest object index, then the lowest triangle index, which is what the object loop keeps. Node boxes are padded, so rounding can't cull a hit. Together this makes renders bit-identical to testing every object; keep all of it when changing the tree, or compare against the object loop. Both render modes use the tree: the SSE path tests a node's 4 child boxes and a leaf's pack at once, the scalar path one by one. Traversal goes straight into the nearest hit child and stacks the others; with closest-hit rays that's what made BVH4 about 20% faster than the binary tree with SSE, where pushing every child had not been faster.

**SIMD path**: `RTTriangle::intersectPack` tests one ray against 4 triangles at a time, for `RTObject::intersect(..., useSIMD)` and the BVH leaves, using `RTVectorPack`/`RTRayPack` ([math/rt_vector_pack.h](math/rt_vector_pack.h), [raytracer/rt_ray_pack.h](raytracer/rt_ray_pack.h)). These are SoA `__m128` X/Y/Z packs. Vertex arrays are padded so the vertex count is a multiple of 12 (4 triangles × 3 vertices). Any change to the mesh layout must keep the scalar and packed paths in sync. Do `__m128` math with `_mm_*` intrinsics, not operators: GCC and Clang accept `a * b` on `__m128` (vector extensions), but in MSVC `__m128` is a union and it doesn't compile.

**Math types**: the core uses its own `RTVector` ([math/rt_vector.h](math/rt_vector.h)). Only the GUI uses Qt's `QVector3D`/`QMatrix4x4`; `GLWidget` builds its projection matrix from `RTCamera`'s getters. `RTCamera` ([scene/rt_camera.h](scene/rt_camera.h)) generates the primary rays. Its private `normalize()` repeats the double-precision math of `QVector3D::normalize()` on purpose, which keeps renders bit-identical to the old Qt-based camera. Don't replace it with `RTVector::Normalize()`.

**Scene files**: `RTScene::saveScene( path )` writes them, and `RTScene::openScene( path, error )` adds the camera, objects and lights of one; on failure, `error` gives the file and line. The GUI's Save Scene writes `scene_<timestamp>.sc` to the current directory. It's a line-based text format:
- `c eye.xyz at.xyz`
- `o type pos.xyz scale.xyz color.rgb material diffuse specular specExp reflection smooth`
- `l type pos.xyz scale.xyz color.rgb intensity` (for a distant light, type 1, `pos.xyz` holds its direction)
- `# comment`: a line whose first non-blank character is `#` is skipped. Blank lines are skipped too. `saveScene` doesn't write comments, so saving a loaded scene drops them.

The type and material values are the integer values of the `RTObjectType`/`RTMaterialType`/light-type enums, so reordering those enums breaks existing scene files. Custom `.obj` objects (type 7) are saved, but loading them fails because the file doesn't store their path. [scenes/example.sc](scenes/example.sc) uses every primitive and material.

**Headless use**: nothing in the core depends on Qt or the UI. To render without the GUI, load a scene with `RTScene::openScene`, render it with `RTRenderer`, and save it with `RTImage::savePNG`, as [cli/main.cpp](cli/main.cpp) does. An MCP server can build on the same pieces.

## Code style

C++11, and Qt in the GUI only. Classes use the `RT`/`GL` prefixes, members are PascalCase without a prefix (`ActiveObject`, `MaxTraceDepth`), there are spaces inside parentheses (`foo( a, b )`), and braces go on their own line.
