#-------------------------------------------------
#
# Project created by QtCreator 2014-03-05T11:43:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = raytracer
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11 -msse -msse4.1

SOURCES += main.cpp\
        mainwindow.cpp \
    image_viewer.cpp \
    manipulators/manipulator.cpp \
    opengl/gl_object.cpp \
    math/utils.cpp \
    math/rt_vector.cpp \
    objects/rt_object.cpp \
    objects/rt_light.cpp \
    objects/rt_distant_light.cpp \
    objects/rt_point_light.cpp \
    raytracer/rt_ray.cpp \
    raytracer/rt_image_parts.cpp \
    raytracer/rt_tracer.cpp \
    rt_scene.cpp \
    rt_camera.cpp \
    opengl/gl_grid.cpp \
    opengl/gl_widget.cpp \
    math/rt_vector_pack.cpp \
    raytracer/rt_ray_pack.cpp \
    3rd_party/tiny_obj_loader.cc

HEADERS  += mainwindow.h \
    image_viewer.h \
    manipulators/manipulator.h \
    opengl/gl_object.h \
    math/rt_vector.h \
    math/utils.h \
    math/rt_vector.h \
    objects/rt_object.h \
    objects/rt_light.h \
    objects/rt_distant_light.h \
    objects/rt_point_light.h \
    raytracer/rt_ray.h \
    raytracer/rt_image_parts.h \
    raytracer/rt_tracer.h \
    rt_scene.h \
    rt_camera.h \
    opengl/gl_grid.h \
    opengl/gl_widget.h \
    math/rt_vector_pack.h \
    raytracer/rt_ray_pack.h \
    3rd_party/tiny_obj_loader.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
