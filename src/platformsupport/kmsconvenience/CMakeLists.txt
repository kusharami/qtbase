# Generated from kmsconvenience.pro.

qt_find_package(Libdrm) # special case

#####################################################################
## KmsSupport Module:
#####################################################################

qt_add_module(KmsSupport
    STATIC
    INTERNAL_MODULE
    SOURCES
        qkmsdevice.cpp qkmsdevice_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Libdrm::Libdrm
        Qt::Core
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:kmsconvenience.pro:<TRUE>:
# MODULE = "kms_support"
