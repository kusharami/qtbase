# Generated from addressbook.pro.

cmake_minimum_required(VERSION 3.14)
project(addressbook LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Widgets)

add_qt_gui_executable(addressbook
    adddialog.cpp adddialog.h
    addresswidget.cpp addresswidget.h
    main.cpp
    mainwindow.cpp mainwindow.h
    newaddresstab.cpp newaddresstab.h
    tablemodel.cpp tablemodel.h
)
target_link_libraries(addressbook PUBLIC
    Qt::Widgets
)

install(TARGETS addressbook
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
