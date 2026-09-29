"""MCP server for the ray tracer: an AI assistant builds a scene with its tools and renders it.

The scene lives in the C++ core. This server loads the raytracer_mcp library (rt_mcp_api.cpp, built by
CMake) with ctypes and calls its C API. Run it with the virtual environment's Python:

    mcp_server/.venv/Scripts/python.exe mcp_server/server.py    (Linux: mcp_server/.venv/bin/python)

It loads the library from build/release, or build/debug if there is no release build.
RAYTRACER_MCP_LIBRARY overrides that with the path of the library file.
"""

import ctypes
import json
import os
import shutil
import sys
import tempfile
import threading
from pathlib import Path
from typing import Annotated, Any, Literal

from pydantic import Field

from mcp.server import MCPServer
from mcp.server.mcpserver import Image
from mcp.server.mcpserver.exceptions import ToolError
from mcp.types import ToolAnnotations

REPOSITORY = Path(__file__).resolve().parent.parent
MESH_DIRECTORY = REPOSITORY / "resources" / "obj"
LIBRARY_FILE = "raytracer_mcp.dll" if sys.platform == "win32" else "raytracer_mcp.so"

# Argument and result types of the C API's functions. Every function but rt_create takes the scene first.
_SCENE = ctypes.c_void_p
_TEXT = ctypes.c_char_p
_INT = ctypes.c_int
_FLOAT = ctypes.c_float
_VECTOR = [_FLOAT] * 3
C_API = {
    "rt_create": ([_TEXT], _SCENE),
    "rt_destroy": ([_SCENE], None),
    "rt_error": ([_SCENE], _TEXT),
    "rt_clear": ([_SCENE], _INT),
    "rt_remove": ([_SCENE, _TEXT], _INT),
    "rt_scene_json": ([_SCENE], _TEXT),
    "rt_item_json": ([_SCENE, _TEXT], _TEXT),
    "rt_set_camera": ([_SCENE, *_VECTOR, *_VECTOR, _FLOAT], _INT),
    "rt_add_object": ([_SCENE, _TEXT, _TEXT], _TEXT),
    "rt_set_object_position": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_object_scale": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_object_color": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_object_material": ([_SCENE, _TEXT, _TEXT], _INT),
    "rt_set_object_diffuse": ([_SCENE, _TEXT, _FLOAT], _INT),
    "rt_set_object_specular": ([_SCENE, _TEXT, _FLOAT], _INT),
    "rt_set_object_specular_exponent": ([_SCENE, _TEXT, _FLOAT], _INT),
    "rt_set_object_reflection": ([_SCENE, _TEXT, _FLOAT], _INT),
    "rt_set_object_smooth_shading": ([_SCENE, _TEXT, _INT], _INT),
    "rt_add_point_light": ([_SCENE, _TEXT, *_VECTOR], _TEXT),
    "rt_add_distant_light": ([_SCENE, _TEXT, *_VECTOR], _TEXT),
    "rt_set_light_position": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_light_direction": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_light_color": ([_SCENE, _TEXT, *_VECTOR], _INT),
    "rt_set_light_intensity": ([_SCENE, _TEXT, _FLOAT], _INT),
    "rt_render": ([_SCENE, _INT, _INT, _INT], _INT),
    "rt_image_data": ([_SCENE], ctypes.c_void_p),
    "rt_image_size": ([_SCENE], ctypes.c_size_t),
    "rt_render_time": ([_SCENE], ctypes.c_double),
}

# The C API function that sets each property
OBJECT_SETTERS = {
    "position": "rt_set_object_position",
    "scale": "rt_set_object_scale",
    "color": "rt_set_object_color",
    "material": "rt_set_object_material",
    "diffuse": "rt_set_object_diffuse",
    "specular": "rt_set_object_specular",
    "specular_exponent": "rt_set_object_specular_exponent",
    "reflection": "rt_set_object_reflection",
    "smooth_shading": "rt_set_object_smooth_shading",
}
LIGHT_SETTERS = {
    "position": "rt_set_light_position",
    "direction": "rt_set_light_direction",
    "color": "rt_set_light_color",
    "intensity": "rt_set_light_intensity",
}


def load_library() -> ctypes.CDLL:
    override = os.environ.get("RAYTRACER_MCP_LIBRARY")
    candidates = [Path(override)] if override else [REPOSITORY / "build" / build / LIBRARY_FILE for build in ("release", "debug")]
    path = next((candidate for candidate in candidates if candidate.is_file()), None)
    if path is None:
        sys.exit(f"raytracer MCP server: {' or '.join(map(str, candidates))} not found; build it with: cmake --build build/release")

    if sys.platform == "win32":
        # Windows locks a loaded DLL. Load a copy, so the build can replace the original while the server runs.
        copies = Path(tempfile.gettempdir()) / "raytracer_mcp"
        copies.mkdir(exist_ok=True)
        for old_copy in copies.glob("*.dll"):
            try:
                old_copy.unlink()
            except OSError:
                pass  # a running server has it loaded
        copy = copies / f"raytracer_mcp_{os.getpid()}.dll"
        shutil.copyfile(path, copy)
        path = copy

    return ctypes.CDLL(str(path))


class Raytracer:
    """One scene in the C++ core, used through the C API of rt_mcp_api.cpp.

    The C API isn't thread-safe, and MCPServer runs each tool call on a worker thread, where calls can
    overlap, so the tools hold `lock` while they use it. ctypes releases the GIL during a call, so the
    server keeps answering while a render runs.
    """

    def __init__(self, library: ctypes.CDLL, mesh_directory: Path):
        self.lock = threading.Lock()
        self._library = library
        for name, (argument_types, result_type) in C_API.items():
            function = getattr(library, name)
            function.argtypes = argument_types
            function.restype = result_type

        # The core opens files with narrow strings, which Windows reads in the ANSI code page
        encoding = "mbcs" if sys.platform == "win32" else sys.getfilesystemencoding()
        self._scene = library.rt_create(str(mesh_directory).encode(encoding))
        if not self._scene:
            raise MemoryError("rt_create failed")

    def call(self, name: str, *arguments: Any) -> Any:
        """Calls a C API function on the scene. Strings go both ways as UTF-8. If the function fails,
        this raises ToolError with rt_error()'s message, which the model gets as the tool's error."""
        function = getattr(self._library, name)
        result = function(self._scene, *(a.encode() if isinstance(a, str) else a for a in arguments))
        if (function.restype is _TEXT and result is None) or (function.restype is _INT and result == 0):
            raise ToolError(self._library.rt_error(self._scene).decode(errors="replace") or "unknown error")
        return result.decode() if function.restype is _TEXT else result

    def set_properties(self, setters: dict[str, str], name: str, properties: dict[str, Any]) -> None:
        """Sets the properties that aren't None, with the C API functions in setters"""
        for key, value in properties.items():
            if value is not None:
                self.call(setters[key], name, *(value if isinstance(value, tuple) else (value,)))

    def item(self, name: str) -> dict[str, Any]:
        return json.loads(self.call("rt_item_json", name))

    def scene(self) -> dict[str, Any]:
        return json.loads(self.call("rt_scene_json"))

    def image(self) -> bytes:
        """The PNG file of the last render"""
        return ctypes.string_at(self.call("rt_image_data"), self.call("rt_image_size"))


INSTRUCTIONS = """\
Builds a 3D scene in a CPU ray tracer and renders it. The scene stays between calls: add objects and \
lights, render to see the result, change things and render again.

- y is up. With the default camera, +x points right and +z toward the viewer.
- Objects are triangle-mesh shapes (see add_object), moved with position and sized per axis with scale. \
There is no rotation. add_object, update_object and get_scene return each object's world-space bounds.
- There is no ambient light: surfaces that no light reaches are black. Shadows are hard, and point lights \
don't fade with distance. The light of all lights adds up and clips at white: one light of intensity 1 \
shows a surface that faces it in the surface's own color.
- Rays that hit nothing show a dark grey background (0.18).
"""

if not MESH_DIRECTORY.is_dir():
    sys.exit(f"raytracer MCP server: mesh folder not found: {MESH_DIRECTORY}")
raytracer = Raytracer(load_library(), MESH_DIRECTORY)
mcp = MCPServer("raytracer", instructions=INSTRUCTIONS)

Vector = tuple[float, float, float]
Fraction = Annotated[float, Field(ge=0, le=1)]
Color = tuple[Fraction, Fraction, Fraction]
Name = Annotated[str, Field(pattern=r"^[A-Za-z0-9_.-]{1,64}$")]
Shape = Literal["plane", "sphere", "cube", "pyramid", "cylinder", "cone", "torus"]
Material = Literal["diffuse", "specular", "diffuse_and_specular", "reflective"]
Intensity = Annotated[float, Field(ge=0, description="Brightness; the light's color is multiplied by it.")]
NewName = Annotated[Name | None, Field(description="Unique name to refer to it later (letters, digits, _ - .); generated if left out.")]

# Parameter descriptions shared by add_object and update_object
POSITION = "Where the shape's origin goes; add_object's description says where each shape sits relative to it."
SCALE = "Size factor per axis (x, y, z); not 0. Negative on all three axes turns the object inside out and mirrors it through position."
COLOR = "RGB, each from 0 to 1."
DIFFUSE = "Weight of the matte part, diffuse_and_specular only."
SPECULAR = "Weight of the highlight, diffuse_and_specular only."
SPECULAR_EXPONENT = "Highlight size: 2 is broad, 50 is a small sharp spot. specular and diffuse_and_specular only."
REFLECTION = "How much of the mirrored image shows, reflective only."
SMOOTH_SHADING = "Interpolate normals so curved shapes look smooth instead of faceted. Not used by reflective."


@mcp.tool(annotations=ToolAnnotations(idempotent_hint=True))
def set_camera(
    eye: Annotated[Vector, Field(description="Where the camera is.")],
    look_at: Annotated[Vector, Field(description="The point in the middle of the image.")],
    fov: Annotated[float, Field(ge=1, le=170, description="Vertical field of view in degrees.")] = 45.0,
) -> dict[str, Any]:
    """Place the camera. The image stays upright (y up); looking straight down, its top points to -z."""
    with raytracer.lock:
        raytracer.call("rt_set_camera", *eye, *look_at, fov)
    return {"eye": eye, "look_at": look_at, "fov": fov}


@mcp.tool()
def add_object(
    shape: Shape,
    position: Annotated[Vector, Field(description=POSITION)] = (0.0, 0.0, 0.0),
    scale: Annotated[Vector, Field(description=SCALE)] = (1.0, 1.0, 1.0),
    color: Annotated[Color, Field(description=COLOR)] = (0.5, 0.5, 0.5),
    material: Material = "diffuse",
    diffuse: Annotated[Fraction, Field(description=DIFFUSE)] = 0.5,
    specular: Annotated[Fraction, Field(description=SPECULAR)] = 0.5,
    specular_exponent: Annotated[float, Field(gt=0, le=10000, description=SPECULAR_EXPONENT)] = 2.0,
    reflection: Annotated[Fraction, Field(description=REFLECTION)] = 0.8,
    smooth_shading: Annotated[bool, Field(description=SMOOTH_SHADING)] = True,
    name: NewName = None,
) -> dict[str, Any]:
    """Add a shape to the scene. Returns the object with its world-space bounds.

    Shapes at scale 1, relative to position:
    - plane: 2x2 square at position's height. One-sided: from below it's shaded as its top side,
      so build walls and ceilings from thin cubes.
    - cube: 1x1x1, standing on position.
    - sphere: diameter 1, resting on position (its center is 0.5 above it).
    - pyramid: triangular (4 faces), base 2 wide and 2 deep on position (one corner toward +z), height 1.
    - cylinder: radius 1, height 2, standing on position.
    - cone: radius 1, height 2, base on position, tip up.
    - torus: flat ring, outer radius 1.25, tube radius 0.25, lying on position but centered 1.05 toward +z
      (subtract 1.05 * scale z from position z to center it).

    Materials:
    - diffuse: matte, in its color.
    - specular: matte plus a highlight in the light's color; specular_exponent sets its size.
    - diffuse_and_specular: the matte part times diffuse plus the highlight times specular.
    - reflective: a mirror that shows reflection times what it reflects. Its own color is ignored,
      it's flat-shaded, and it needs things around it to reflect (see render's max_depth).
    """
    with raytracer.lock:
        name = raytracer.call("rt_add_object", name or "", shape)
        properties = {
            "position": position, "scale": scale, "color": color, "material": material, "diffuse": diffuse,
            "specular": specular, "specular_exponent": specular_exponent, "reflection": reflection,
            "smooth_shading": smooth_shading,
        }
        try:
            raytracer.set_properties(OBJECT_SETTERS, name, properties)
        except ToolError:
            raytracer.call("rt_remove", name)
            raise
        return raytracer.item(name)


@mcp.tool()
def update_object(
    name: Annotated[str, Field(description="The object's name.")],
    position: Annotated[Vector | None, Field(description=POSITION)] = None,
    scale: Annotated[Vector | None, Field(description=SCALE)] = None,
    color: Annotated[Color | None, Field(description=COLOR)] = None,
    material: Material | None = None,
    diffuse: Annotated[Fraction | None, Field(description=DIFFUSE)] = None,
    specular: Annotated[Fraction | None, Field(description=SPECULAR)] = None,
    specular_exponent: Annotated[float | None, Field(gt=0, le=10000, description=SPECULAR_EXPONENT)] = None,
    reflection: Annotated[Fraction | None, Field(description=REFLECTION)] = None,
    smooth_shading: Annotated[bool | None, Field(description=SMOOTH_SHADING)] = None,
) -> dict[str, Any]:
    """Change an object; leave out what stays the same. Returns the object with its world-space bounds."""
    properties = {
        "position": position, "scale": scale, "color": color, "material": material, "diffuse": diffuse,
        "specular": specular, "specular_exponent": specular_exponent, "reflection": reflection,
        "smooth_shading": smooth_shading,
    }
    if all(value is None for value in properties.values()):
        raise ToolError("nothing to change: give at least one property")
    with raytracer.lock:
        raytracer.set_properties(OBJECT_SETTERS, name, properties)
        return raytracer.item(name)


@mcp.tool()
def add_point_light(
    position: Annotated[Vector, Field(description="Where the light is.")],
    color: Annotated[Color, Field(description=COLOR)] = (1.0, 1.0, 1.0),
    intensity: Intensity = 1.0,
    name: NewName = None,
) -> dict[str, Any]:
    """Add a point light, which shines from a point in every direction and doesn't fade with distance."""
    with raytracer.lock:
        name = raytracer.call("rt_add_point_light", name or "", *position)
        try:
            raytracer.set_properties(LIGHT_SETTERS, name, {"color": color, "intensity": intensity})
        except ToolError:
            raytracer.call("rt_remove", name)
            raise
        return raytracer.item(name)


@mcp.tool()
def add_distant_light(
    direction: Annotated[Vector, Field(description="The way the light travels, e.g. (0, -1, 0) shines straight down.")],
    color: Annotated[Color, Field(description=COLOR)] = (1.0, 1.0, 1.0),
    intensity: Intensity = 1.0,
    name: NewName = None,
) -> dict[str, Any]:
    """Add a distant light, like the sun: parallel rays in one direction, equally bright everywhere."""
    with raytracer.lock:
        name = raytracer.call("rt_add_distant_light", name or "", *direction)
        try:
            raytracer.set_properties(LIGHT_SETTERS, name, {"color": color, "intensity": intensity})
        except ToolError:
            raytracer.call("rt_remove", name)
            raise
        return raytracer.item(name)


@mcp.tool()
def update_light(
    name: Annotated[str, Field(description="The light's name.")],
    position: Annotated[Vector | None, Field(description="New position of a point light.")] = None,
    direction: Annotated[Vector | None, Field(description="New direction of a distant light.")] = None,
    color: Annotated[Color | None, Field(description=COLOR)] = None,
    intensity: Annotated[float | None, Field(ge=0, description="Brightness; the light's color is multiplied by it.")] = None,
) -> dict[str, Any]:
    """Change a light; leave out what stays the same. Returns the light."""
    properties = {"position": position, "direction": direction, "color": color, "intensity": intensity}
    if all(value is None for value in properties.values()):
        raise ToolError("nothing to change: give at least one property")
    with raytracer.lock:
        raytracer.set_properties(LIGHT_SETTERS, name, properties)
        return raytracer.item(name)


@mcp.tool(annotations=ToolAnnotations(destructive_hint=True))
def remove(name: Annotated[str, Field(description="Name of an object or light.")]) -> str:
    """Remove an object or light."""
    with raytracer.lock:
        raytracer.call("rt_remove", name)
    return f"Removed {name}."


@mcp.tool(annotations=ToolAnnotations(destructive_hint=True, idempotent_hint=True))
def clear_scene() -> str:
    """Remove every object and light. The camera stays."""
    with raytracer.lock:
        raytracer.call("rt_clear")
    return "The scene is empty."


@mcp.tool(annotations=ToolAnnotations(read_only_hint=True))
def get_scene() -> dict[str, Any]:
    """The camera, objects and lights, with each object's world-space bounds and triangle count."""
    with raytracer.lock:
        return raytracer.scene()


@mcp.tool()
def render(
    width: Annotated[int, Field(ge=16, le=2048, description="Image width in pixels.")] = 800,
    height: Annotated[int, Field(ge=16, le=2048, description="Image height in pixels.")] = 600,
    max_depth: Annotated[int, Field(ge=1, le=10, description="Rays bounce off mirrors up to max_depth - 1 times; deeper reflections are black.")] = 3,
    save_path: Annotated[str | None, Field(description="Also save the PNG here, overwriting an existing file. It must end in .png; a relative path is relative to the repository.")] = None,
) -> list[Image | str]:
    """Render the scene as it is now and return the image. Call it whenever you want to check progress;
    small images such as 400x300 are quicker."""
    if save_path is not None and not save_path.lower().endswith(".png"):
        raise ToolError("save_path must end in .png")

    with raytracer.lock:
        raytracer.call("rt_render", width, height, max_depth)
        png = raytracer.image()
        seconds = raytracer.call("rt_render_time")
        scene = raytracer.scene()

    objects, lights = scene["objects"], scene["lights"]
    summary = (f"Rendered {width}x{height} in {seconds:.2f} s: {len(objects)} objects "
               f"({sum(o['triangles'] for o in objects)} triangles), {len(lights)} lights.")
    if not objects:
        summary += " The scene is empty."
    elif not lights:
        summary += " Without lights every object is black."

    if save_path is not None:
        path = REPOSITORY / save_path  # an absolute save_path replaces REPOSITORY
        try:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(png)
        except OSError as error:
            raise ToolError(f"cannot save {path}: {error}") from error
        summary += f" Saved to {path}."

    return [Image(data=png, format="png"), summary]


if __name__ == "__main__":
    mcp.run()
