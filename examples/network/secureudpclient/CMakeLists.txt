# Generated from secureudpclient.pro.

cmake_minimum_required(VERSION 3.14)
project(secureudpclient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Network)

add_qt_gui_executable(secureudpclient
    addressdialog.cpp addressdialog.h addressdialog.ui
    association.cpp association.h
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)
target_link_libraries(secureudpclient PUBLIC
    Qt::Network
    Qt::Widgets
)

install(TARGETS secureudpclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
