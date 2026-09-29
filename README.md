<p align="center">
  <img width="914" height="344" src="https://raw.githubusercontent.com/mandyedi/raytracing-thesis/master/renderings.png">
</p>

This project began as my university thesis: a CPU ray tracer written in C++ and Qt, sped up with multithreading and SSE. If you want the original version, see the [`thesis-release`](https://github.com/mandyedi/raytracing-thesis/tree/thesis-release) branch.

The project now continues as an experiment: building an MCP (Model Context Protocol) server for the ray tracer, so that AI assistants can set up scenes and render ray-traced images.

## Build

You need CMake 3.16 or newer and a C++11 compiler for x86-64. The Qt 5 GUI is built as well if Qt 5 is found.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release    # MSYS2 MinGW: add -G "MinGW Makefiles"
cmake --build build/release
```

## Command line renderer

`raytracer-cli` renders a [scene file](#scene-files) to a PNG image, without Qt:

```sh
build/release/raytracer-cli scenes/example.sc -o example.png
```

It reads the meshes from the `resources` folder next to it, which the build puts there. Run `raytracer-cli --help` for the options: image size, threads, trace depth and SSE.

## MCP server

The MCP server lets an AI assistant such as Claude Code build a scene and render it. It doesn't use scene files: the assistant adds objects and lights with the server's tools, and it can render at any time to see how the scene looks so far. The server is written in Python with the official [MCP Python SDK](https://github.com/modelcontextprotocol/python-sdk), while the ray tracing runs in C++: [mcp_server/server.py](mcp_server/server.py) calls the core through the C API in [mcp_server/rt_mcp_api.cpp](mcp_server/rt_mcp_api.cpp), which the build compiles into the `raytracer_mcp` library.

To set it up, run this in the repository folder. You need Python 3.10 or newer.

```sh
python -m venv mcp_server/.venv
mcp_server/.venv/Scripts/python -m pip install -r mcp_server/requirements.txt    # Linux: mcp_server/.venv/bin/python
cmake --build build/release
```

[.mcp.json](.mcp.json) adds the server to Claude Code for this project. Claude Code asks you to approve it the first time, and `/mcp` shows whether it's connected. On Linux, change `Scripts/python.exe` to `bin/python` in `.mcp.json`. Other MCP clients start it the same way, with the virtual environment's Python and `mcp_server/server.py`, and talk to it over stdio.

The server loads `raytracer_mcp` from `build/release`, or from `build/debug` if there's no release build. To use another build, set `RAYTRACER_MCP_LIBRARY` to the library's path. On Windows the server loads a copy of the library, so you can rebuild while it runs; restart the server to use the new build.

| Tool | What it does |
| --- | --- |
| `set_camera` | Places the camera: where it is, the point it looks at and its vertical field of view. |
| `add_object` | Adds a shape (plane, sphere, cube, pyramid, cylinder, cone or torus) with a position, scale, color and material. |
| `update_object` | Changes some of an object's properties. |
| `add_point_light`, `add_distant_light` | Adds a light. |
| `update_light` | Changes some of a light's properties. |
| `remove`, `clear_scene` | Removes one object or light, or all of them. |
| `get_scene` | Returns the camera, objects and lights, with each object's bounding box. |
| `render` | Renders the scene and returns the PNG image. It can also save it to a file. |

Objects and lights have names, which the assistant chooses or the server generates. Objects, materials and lights work as in [scene files](#scene-files), with two differences: the camera keeps the image upright however far it looks up or down, even straight down, and distant light directions are normalized.

## Scene files

A scene file (`.sc`) is a text file with one item per line: the first value says what the line adds, `c` for the camera, `o` for an object or `l` for a light. The values after it are separated by spaces and always come in the same order. The GUI's Save Scene writes these files, and you can also write them by hand. [scenes/example.sc](scenes/example.sc) uses every object type and material.

- Every value on a line is required, even the ones its material doesn't use.
- `type`, `material` and `smooth` are integers: write `2`, not `2.0`.
- Blank lines are skipped. There are no comments: a line that starts with anything other than `c`, `o` or `l` is an error.
- +y is up. With the default camera, +x points right and +z toward the viewer.
- Colors are RGB values from 0 to 1.
- The background is always dark gray (0.18).

This scene has a gray floor, a shiny blue sphere, a mirror cube, a white point light and a warm distant light that shines straight down:

```text
c 0 2 6 0 0.5 0
o 0 0 0 0 4 1 4 0.5 0.5 0.5 0 1 0 2 0 0
o 1 -1 0 0 1.5 1.5 1.5 0.125 0.5 0.75 2 0.75 0.25 32 0 1
o 2 1.25 0 0 1 1 1 1 1 1 3 0.5 0.5 2 0.8 0
l 0 -3 5 4 0.05 0.05 0.05 1 1 1 0.75
l 1 0 -1 0 0.05 0.05 0.05 1 0.9 0.8 0.25
```

### Camera

```text
c eye.x eye.y eye.z at.x at.y at.z
```

| Value | Meaning |
| --- | --- |
| `eye` | Where the camera is. |
| `at` | The point the camera looks at. |

It's a perspective camera with a 45° vertical field of view. Its up direction is always +y and doesn't tilt with the view, so keep the camera close to level: the more it looks up or down, the more the image is zoomed in and distorted, and looking straight up or down doesn't work. If there's more than one camera line, the last one wins. Without a camera line, `raytracer-cli` looks from `-0.8 1.6 6` at `0 0 0`, and the GUI keeps its current camera.

### Objects

```text
o type pos.x pos.y pos.z scale.x scale.y scale.z color.r color.g color.b material diffuse specular specExp reflection smooth
```

| Value | Range | Meaning |
| --- | --- | --- |
| `type` | 0–6 | The shape, see [Object types](#object-types). |
| `pos` | | Where the mesh's origin goes. [Object types](#object-types) says where the origin is on each shape. |
| `scale` | | Scale along x, y and z. Objects can't be rotated. |
| `color` | 0–1 | Surface color. Reflective objects ignore it. |
| `material` | 0–3 | How the surface is shaded, see [Materials](#materials). |
| `diffuse` | 0–1 | Weight of the diffuse shading. Only Diffuse and specular uses it. |
| `specular` | 0–1 | Weight of the highlights. Only Diffuse and specular uses it. The GUI sets it to 1 − `diffuse`. |
| `specExp` | 2–2000 | Specular exponent: the higher it is, the smaller and sharper the highlights. Specular and Diffuse and specular use it. |
| `reflection` | 0–1 | How much a Reflective object reflects: 1 is a perfect mirror, 0 is black. |
| `smooth` | 0 or 1 | 1 is smooth shading, which interpolates the vertex normals; 0 shades each triangle flat. Reflective objects ignore it and always reflect off the flat triangles. |

The ranges of `type` and `material` are checked when the scene loads. The other ranges are the ones the GUI allows; the file accepts any number.

#### Object types

Every shape is a triangle mesh from the `resources/obj` folder. The sizes are at scale 1.

| `type` | Shape | Size and origin |
| --- | --- | --- |
| 0 | Plane | A 2 × 2 square in the xz-plane, centered on `pos`. |
| 1 | Sphere | Diameter 1, standing on `pos`. |
| 2 | Cube | 1 × 1 × 1, standing on `pos`. |
| 3 | Pyramid | A triangular pyramid (4 faces), 2 wide and 1 tall, standing on `pos`. |
| 4 | Cylinder | Diameter 2 and 2 tall, standing on `pos`. |
| 5 | Cone | Base diameter 2 and 2 tall, point up, standing on `pos`. |
| 6 | Torus | Lies flat: ring radius 1, tube radius 0.25. Its center is about 1.05 × `scale.z` in front of `pos` (toward +z). |

Type 7 is a custom `.obj` mesh added in the GUI. The GUI saves it, but a scene file with one can't be loaded, because the file doesn't store the mesh's path.

#### Materials

| `material` | Name | Shading |
| --- | --- | --- |
| 0 | Diffuse | Matte: `color`, lit by each light according to the angle the light hits the surface at. |
| 1 | Specular | Shiny: the Diffuse shading as a base, plus highlights in the lights' color. |
| 2 | Diffuse and specular | `diffuse` × the Diffuse shading + `specular` × the highlights. |
| 3 | Reflective | A mirror: `reflection` × whatever the reflected ray hits. `color` and the lights don't affect it. A mirror seen in a mirror reflects only as deep as the trace depth (`--depth`, default 3); past it, it's black. |

### Lights

```text
l type pos.x pos.y pos.z scale.x scale.y scale.z color.r color.g color.b intensity
```

| Value | Meaning |
| --- | --- |
| `type` | 0 is a point light, 1 is a distant light. |
| `pos` | For a point light, its position. For a distant light, the direction its light travels in: `0 -1 0` shines straight down. Give the direction a length of 1: it isn't normalized, so a longer vector makes the light brighter. |
| `scale` | The size of the light's marker in the GUI's viewport. It doesn't change the render, and loading a scene ignores it; the GUI saves `0.05 0.05 0.05`. |
| `color` | The light's color, 0–1. |
| `intensity` | Brightness: it multiplies `color`. |

A point light shines in all directions from one point and doesn't fade with distance. A distant light is like the sun: its rays are parallel, so its light has the same direction everywhere. Both cast hard shadows. For a point light, only objects between the surface and the light cast them. The lights themselves don't show up in the image, and without any light, only the background and its reflections are visible.
