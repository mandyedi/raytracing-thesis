cmake_minimum_required( VERSION 3.16 )
project( raytracer LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if ( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
    set( CMAKE_BUILD_TYPE Release )
endif()

option( RAYTRACER_BUILD_GUI "Build the Qt 5 GUI (skipped when Qt 5 is not found)" ON )

find_package( Threads REQUIRED )

# Ray tracing core, shared by the CLI and the GUI. It must not depend on Qt.
add_library( raytracer_core STATIC
    math/rt_vector.cpp
    math/rt_vector_pack.cpp
    math/utils.cpp
    objects/rt_object.cpp
    objects/rt_light.cpp
    objects/rt_point_light.cpp
    objects/rt_distant_light.cpp
    raytracer/rt_ray.cpp
    raytracer/rt_ray_pack.cpp
    raytracer/rt_image.cpp
    raytracer/rt_image_parts.cpp
    raytracer/rt_tracer.cpp
    raytracer/rt_renderer.cpp
    scene/rt_camera.cpp
    scene/rt_scene.cpp
    3rd_party/tiny_obj_loader.cc
)
target_include_directories( raytracer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( raytracer_core PUBLIC Threads::Threads )
if ( CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" )
    target_compile_options( raytracer_core PUBLIC -msse -msse4.1 )
endif()

# Command line renderer
add_executable( raytracer-cli cli/main.cpp )
target_link_libraries( raytracer-cli PRIVATE raytracer_core )
if ( MINGW )
    # Runs without the MinGW runtime DLLs on PATH
    target_link_options( raytracer-cli PRIVATE -static )
endif()
# The CLI reads the meshes from the resources folder next to it
add_custom_command( TARGET raytracer-cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/obj
            $<TARGET_FILE_DIR:raytracer-cli>/resources/obj
)

# Qt GUI
if ( RAYTRACER_BUILD_GUI )
    find_package( Qt5 COMPONENTS Core Gui Widgets OpenGL QUIET )
    if ( Qt5_FOUND )
        add_executable( raytracer WIN32
            main.cpp
            gui/mainwindow.cpp
            gui/mainwindow.h
            gui/mainwindow.ui
            gui/image_viewer.cpp
            gui/image_viewer.h
            manipulators/manipulator.cpp
            manipulators/manipulator.h
            opengl/gl_object.cpp
            opengl/gl_grid.cpp
            opengl/gl_widget.cpp
            opengl/gl_widget.h
            resources/resources.qrc
        )
        set_target_properties( raytracer PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON )
        target_link_libraries( raytracer PRIVATE raytracer_core Qt5::Widgets Qt5::OpenGL )
    else()
        message( STATUS "Qt 5 not found, building the CLI only" )
    endif()
endif()
