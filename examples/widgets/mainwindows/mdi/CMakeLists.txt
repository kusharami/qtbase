# Generated from mdi.pro.

cmake_minimum_required(VERSION 3.14)
project(mdi LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Widgets)

add_qt_gui_executable(mdi
    main.cpp
    mainwindow.cpp mainwindow.h
    mdi.qrc
    mdichild.cpp mdichild.h
)
target_link_libraries(mdi PUBLIC
    Qt::Widgets
)

install(TARGETS mdi
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
