# Generated from offscreen.pro.

#####################################################################
## offscreen Binary:
#####################################################################

add_qt_manual_test(offscreen
    SOURCES
        offscreen.cpp
    LIBRARIES
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set_source_files_properties("../shared/color.frag.qsb"
    PROPERTIES QT_RESOURCE_ALIAS "color.frag.qsb"
)
set_source_files_properties("../shared/color.vert.qsb"
    PROPERTIES QT_RESOURCE_ALIAS "color.vert.qsb"
)
set(offscreen_resource_files
    "../shared/color.frag.qsb"
    "../shared/color.vert.qsb"
)

add_qt_resource(offscreen "offscreen"
    PREFIX
        "/"
    FILES
        ${offscreen_resource_files}
)


#### Keys ignored in scope 1:.:.:offscreen.pro:<TRUE>:
# TEMPLATE = "app"
