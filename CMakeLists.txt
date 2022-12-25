cmake_minimum_required(VERSION 3.5)

project(Qt-opengl-phys-enige LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        glsimulation.cpp
        body.cpp
        world.cpp
        camera.cpp
        ShadowMapTech.cpp
)

include_directories(./eigen)

set(RESOURCE_FILES
    resources/*
)

add_executable(Qt-opengl-phys-enige
    ${PROJECT_SOURCES}
)

target_link_libraries(Qt-opengl-phys-enige
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARY}
    -lassimp
    )

#set_target_properties(Qt-opengl-phys-enige PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#    MACOSX_BUNDLE TRUE
#    WIN32_EXECUTABLE TRUE
#)

#if(QT_VERSION_MAJOR EQUAL 6)
#    qt_finalize_executable(untitled)
#endif()
