# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A CPU ray tracer in C++ and Qt (originally a university thesis), sped up with multithreading and SSE. The original thesis version lives on the `thesis-release` branch. `master` is continuing it as an experiment: building an MCP (Model Context Protocol) server so AI assistants can set up scenes and render ray-traced images.

## Build

It's a qmake project ([raytracer.pro](raytracer.pro)). There are no tests, lint config or CI.

```sh
qmake raytracer.pro
make            # or nmake / jom with MSVC; or open raytracer.pro in Qt Creator
```

- Needs **Qt 5** (modules: core, gui, opengl, widgets). The viewport uses `QGLWidget`, which was removed in Qt 6.
- The compiler flags `-std=c++11 -msse -msse4.1` are GCC/Clang style (MinGW on Windows). With MSVC they need to be adapted.
- The build only compiles files listed in `SOURCES`/`HEADERS` in the `.pro`. When you add a file, add it there too. Some files in the repo are **not** built: `cpu_info_window.*` and `raytracer/rt_packed_data.cpp` (an old standalone SSE scratch test that includes `stdafx.h`).
- The `.obj` meshes and GLSL shaders are embedded through [resources.qrc](resources.qrc) (`:/obj/...`, `:/shaders/...`).

## Architecture

**Two parallel representations of each object.** `RTObject` ([objects/rt_object.h](objects/rt_object.h)) holds the ray tracing data: triangle vertices, face and vertex normals, and SSE-packed vertices. It owns a `GLObject` ([opengl/gl_object.h](opengl/gl_object.h)) that draws the same mesh in the interactive OpenGL viewport (`GLWidget`). Lights (`RTLight` → `RTPointLight`, `RTDistantLight`) follow the same pattern. Every primitive (sphere, cube, torus, …) is a triangle mesh. `RTScene::getResourceFile` copies it out of the Qt resource into a temp dir, and tinyobjloader loads it (only the first shape is used). There are no analytic primitives.

**Rendering flow** (in [gui/mainwindow.cpp](gui/mainwindow.cpp): `on_renderButton_clicked` = scalar, `on_pushSSEButton_clicked` = SSE; the two are near-duplicates):
1. Allocate an `RTVector**` pixel buffer.
2. Split the image into tiles in `RTImageParts`, a mutex-guarded work queue.
3. Create N `RTTracer` QObjects, each `moveToThread` to its own `QThread`. Each thread pops tiles until the queue is empty. The UI thread blocks on `wait()`.
4. `ImageViewer` shows the buffer and saves `savedFromProg_<timestamp>.png` in the current directory.

**Tracer** ([raytracer/rt_tracer.cpp](raytracer/rt_tracer.cpp)): `castRay` brute-forces every object (`Trace`), which means every triangle; there is no acceleration structure. It shades based on `RTMaterialType` (Diffuse, Specular, DiffuseAndSpecular, Reflective), casts hard shadow rays, and recurses for reflection up to `MaxTraceDepth`. Smooth shading interpolates vertex normals with the barycentric `u, v` from the triangle hit.

**SIMD path**: `RTObject::intersect(..., useSIMD)` tests one ray against 4 triangles at a time, using `RTVectorPack`/`RTRayPack` ([math/rt_vector_pack.h](math/rt_vector_pack.h), [raytracer/rt_ray_pack.h](raytracer/rt_ray_pack.h)). These are SoA `__m128` X/Y/Z packs. Vertex arrays are padded so the vertex count is a multiple of 12 (4 triangles × 3 vertices). Any change to the mesh layout must keep the scalar and packed paths in sync.

**Math types**: the ray tracer uses its own `RTVector` ([math/rt_vector.h](math/rt_vector.h)). The camera and GL side use Qt's `QVector3D`/`QMatrix4x4`. `RTCamera` bridges the two by generating the primary rays.

**Scene files**: `RTScene::saveScene` writes `scene_<timestamp>.sc` to the current directory, and `openScene` reads it back and also fills the UI list widget. It's a line-based text format:
- `c eye.xyz at.xyz`
- `o type pos.xyz scale.xyz color.rgb material diffuse specular specExp reflection smooth`
- `l type pos.xyz scale.xyz color.rgb intensity`

The type and material values are the integer values of the `RTObjectType`/`RTMaterialType`/light-type enums, so reordering those enums breaks existing scene files.

**Coupling to note**: `RTScene::openScene` takes `Ui::MainWindow*`, and the rendering is started from UI slots. The scene and tracer are not yet usable headless. An MCP server (or any non-GUI entry point) will need to separate them from `MainWindow`.

## Code style

Qt/C++11. Classes use the `RT`/`GL` prefixes, members are PascalCase without a prefix (`ActiveObject`, `MaxTraceDepth`), there are spaces inside parentheses (`foo( a, b )`), and braces go on their own line.
