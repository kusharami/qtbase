# Generated from fetchmore.pro.

cmake_minimum_required(VERSION 3.14)
project(fetchmore LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Widgets)

add_qt_gui_executable(fetchmore
    filelistmodel.cpp filelistmodel.h
    main.cpp
    window.cpp window.h
)
target_link_libraries(fetchmore PUBLIC
    Qt::Widgets
)

install(TARGETS fetchmore
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
