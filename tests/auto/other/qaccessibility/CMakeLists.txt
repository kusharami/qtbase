# Generated from qaccessibility.pro.

if(NOT QT_FEATURE_accessibility)
    return()
endif()

# special case begin
if (WIN32 AND NOT TARGET Qt::WindowsUIAutomationSupport)
    return()
endif()
# special case end

#####################################################################
## tst_qaccessibility Test:
#####################################################################

add_qt_test(tst_qaccessibility
    SOURCES
        accessiblewidgets.h
        tst_qaccessibility.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::TestPrivate
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Widgets
)

#### Keys ignored in scope 1:.:.:qaccessibility.pro:<TRUE>:
# _REQUIREMENTS = "qtConfig(accessibility)"

## Scopes:
#####################################################################

extend_target(tst_qaccessibility CONDITION UNIX AND NOT APPLE AND NOT HAIKU AND NOT integity
    PUBLIC_LIBRARIES
        m
)

extend_target(tst_qaccessibility CONDITION WIN32
    PUBLIC_LIBRARIES
        ole32
        oleacc
        oleaut32
        uuid
)

extend_target(tst_qaccessibility CONDITION WIN32 AND NOT WINRT
    LIBRARIES
        Qt::WindowsUIAutomationSupportPrivate
    PUBLIC_LIBRARIES
        Qt::WindowsUIAutomationSupport
)
