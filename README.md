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

`raytracer-cli` renders a scene file saved by the GUI to a PNG image, without Qt:

```sh
build/release/raytracer-cli scenes/example.sc -o example.png
```

It reads the meshes from the `resources` folder next to it, which the build puts there. Run `raytracer-cli --help` for the options: image size, threads, trace depth and SSE.
