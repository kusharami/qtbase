# Generated from tslib.pro.

qt_find_package(Tslib) # special case

#####################################################################
## QTsLibPlugin Plugin:
#####################################################################

qt_add_plugin(QTsLibPlugin
    OUTPUT_NAME qtslibplugin
    TYPE generic
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::InputSupportPrivate
    PUBLIC_LIBRARIES
        PkgConfig::Tslib
        Qt::Core
        Qt::Gui
        Qt::InputSupport
)

#### Keys ignored in scope 1:.:.:tslib.pro:<TRUE>:
# OTHER_FILES = "tslib.json"
# PLUGIN_EXTENDS = "-"
