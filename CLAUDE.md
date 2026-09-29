# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A CPU ray tracer in C++ (originally a university thesis with a Qt GUI), sped up with multithreading and SSE. The original thesis version lives on the `thesis-release` branch. `master` is continuing it as an experiment: an MCP (Model Context Protocol) server so AI assistants can set up scenes and render ray-traced images. The ray tracing core is Qt-free. A command line renderer (`raytracer-cli`) renders scene files without Qt, and the MCP server ([mcp_server/](mcp_server/)) builds scenes through a C API over the core.

## Build

It's a CMake project ([CMakeLists.txt](CMakeLists.txt)). There are no tests, lint config or CI.

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release    # MSYS2 MinGW: add -G "MinGW Makefiles"
cmake --build build/release
```

- Targets:
  - `raytracer_core`: static library with `math/`, `objects/`, `raytracer/`, `scene/` and tinyobjloader. **It must stay Qt-free.** It doesn't link Qt, so a Qt include there fails to compile.
  - `raytracer-cli` ([cli/main.cpp](cli/main.cpp)): links only the core. MinGW links it with `-static`, so it needs no MinGW DLLs. After each build, `resources/obj` is copied next to it.
  - `raytracer_mcp` ([mcp_server/rt_mcp_api.cpp](mcp_server/rt_mcp_api.cpp)): the MCP server's C API, a shared library (`raytracer_mcp.dll`/`.so`, no `lib` prefix) that links the core. MinGW links it with `-static` too. Because of it the core is compiled with `POSITION_INDEPENDENT_CODE`: `-fPIC` on Linux, which measured 0–1% slower on the larger scenes, and nothing on Windows.
  - `raytracer`: the GUI. It's built only when **Qt 5** is found (Core, Gui, Widgets, OpenGL); `-DRAYTRACER_BUILD_GUI=OFF` skips it. The viewport uses `QGLWidget`, which was removed in Qt 6.
- C++11, x86-64. GCC/Clang get `-msse -msse4.1`; the code only uses SSE/SSE2 intrinsics, so MSVC needs no flag. Verified with MSYS2 MinGW and MSVC 2022 (CLI only, no Qt for MSVC here) on Windows and GCC on Linux; all three render identical pixels.
- [.vscode/tasks.json](.vscode/tasks.json) builds `build/debug` and `build/release` with MSYS2 MinGW (`C:\msys64\mingw64\bin`).
- The build only compiles files listed in `CMakeLists.txt`. When you add a file, add it there too. Some files in the repo are **not** built: `cpu_info_window.*` and `raytracer/rt_packed_data.cpp` (an old standalone SSE scratch test that includes `stdafx.h`).
- Meshes and shaders: [resources/resources.qrc](resources/resources.qrc) embeds the `.obj` meshes and GLSL shaders into the GUI (`:/obj/...`, `:/shaders/...`). The subfolders mirror the resource paths: `resources/obj/sphere.obj` is `:/obj/sphere.obj`. The core loads meshes from files, not Qt resources. The GUI copies `:/obj` into a temp folder at startup, and the CLI reads the `resources/obj` folder next to its executable (or `--resources <dir>`).

## Command line

```sh
raytracer-cli scenes/example.sc -o example.png [--width 800] [--height 600] [--threads N] [--depth 3] [--scalar] [--resources <dir>]
```

The defaults match the GUI: 800×600 and trace depth 3. Without `-o`, it writes `<scene name>_<yyyyMMdd_hhmmss>.png` (local time, like the GUI's file names) to the current folder; a path given with `-o` is used as is and overwrites an existing file. It uses all hardware threads, and SSE unless `--scalar` is given (`--scalar` is the GUI's plain Render button). Exit codes: 1 usage error, 2 scene or mesh error, 3 the image can't be written.

## MCP server

[mcp_server/server.py](mcp_server/server.py) is the MCP server, written with the official Python MCP SDK (`mcp==2.2.0` in [mcp_server/requirements.txt](mcp_server/requirements.txt), installed in `mcp_server/.venv`). It loads `raytracer_mcp` with ctypes; [README.md](README.md#mcp-server) has the setup and the tools, and [.mcp.json](.mcp.json) registers it with Claude Code. Rules:

- SDK 2.x differs from the 1.x examples online: `from mcp.server import MCPServer` (there's no `FastMCP`). Plain `def` tools run on worker threads, and only a `ToolError`'s message reaches the model; any other exception shows it just `Error executing tool <name>`.
- The C API isn't thread-safe and tool calls can overlap, so every tool holds `raytracer.lock` while it calls the library.
- A new C function must also go in `C_API` in `server.py`, with its exact argument and result types: a wrong ctypes signature crashes the server instead of raising an error. C API functions catch every C++ exception and return 1/0 (or text/`nullptr`), with the message in `rt_error()`.
- The stdio transport sends the protocol over stdout, so nothing in the core or the C API may write to stdout. stderr is fine.
- On Windows the server loads a temporary copy of the DLL, so builds can replace it. Restart the server (in Claude Code: `/mcp`) to use a new build.
- The MCP camera gets an up vector perpendicular to the view (`placeCamera` in `rt_mcp_api.cpp`, via `RTCamera::setUp`), so any view is undistorted. Scene files keep the old fixed up of (0, 1, 0), which keeps their renders unchanged.
- To test without an MCP client, drive the server with the SDK's own client, in a script run by the venv's Python: `async with Client(StdioServerParameters(command="mcp_server/.venv/Scripts/python.exe", args=["mcp_server/server.py"])) as client:`, then `await client.call_tool(name, arguments)`.

## Architecture

The architecture is described in [docs/architecture.md](docs/architecture.md): how objects and lights are stored, the rendering flow, the tracer, the BVH, the SIMD path, the math types, the scene file format and headless use. Whenever you need information about the architecture, read [docs/architecture.md](docs/architecture.md) first.

## Code style

C++11, and Qt in the GUI only. Classes use the `RT`/`GL` prefixes, members are PascalCase without a prefix (`ActiveObject`, `MaxTraceDepth`), there are spaces inside parentheses (`foo( a, b )`), and braces go on their own line.
