# Generated from qopenglwidget.pro.

cmake_minimum_required(VERSION 3.14)
project(qopenglwidget LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/opengl/qopenglwidget")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS OpenGL)

add_qt_gui_executable(qopenglwidget
    bubble.cpp bubble.h
    glwidget.cpp glwidget.h
    main.cpp
    mainwindow.cpp mainwindow.h
)
target_link_libraries(qopenglwidget PUBLIC
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::Widgets
)


# Resources:
set(texture_resource_files
    "qt.png"
)

qt6_add_resources(qopenglwidget "texture"
    PREFIX
        "/"
    FILES
        ${texture_resource_files}
)

install(TARGETS qopenglwidget
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
