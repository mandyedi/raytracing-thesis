cmake_minimum_required( VERSION 3.16 )
project( raytracer LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if ( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
    set( CMAKE_BUILD_TYPE Release )
endif()

find_package( Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC_SEARCH_PATHS gui )

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
    math/utils.cpp
    math/rt_vector.cpp
    math/rt_vector_pack.cpp
    objects/rt_object.cpp
    objects/rt_light.cpp
    objects/rt_distant_light.cpp
    objects/rt_point_light.cpp
    raytracer/rt_ray.cpp
    raytracer/rt_ray_pack.cpp
    raytracer/rt_image_parts.cpp
    raytracer/rt_tracer.cpp
    raytracer/rt_tracer.h
    scene/rt_scene.cpp
    scene/rt_camera.cpp
    3rd_party/tiny_obj_loader.cc
    resources/resources.qrc
)
target_include_directories( raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_compile_options( raytracer PRIVATE -msse -msse4.1 )
target_link_libraries( raytracer PRIVATE Qt5::Widgets Qt5::OpenGL )
