/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwasmeventtranslator.h"
#include "qwasmeventdispatcher.h"
#include "qwasmcompositor.h"
#include "qwasmintegration.h"
#include "qwasmclipboard.h"
#include "qwasmstring.h"

#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>
#include <private/qguiapplication_p.h>
#include <qpa/qwindowsysteminterface.h>
#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>

#include <QtCore/qdeadlinetimer.h>
#include <private/qmakearray_p.h>
#include <QtCore/qnamespace.h>

#include <emscripten/bind.h>

#include <iostream>
#include <set>

using namespace emscripten;

QT_BEGIN_NAMESPACE

typedef struct emkb2qt {
    const char *em;
    unsigned int qt;

    constexpr bool operator <=(const emkb2qt &that) const noexcept
    {
        return !(strcmp(that) > 0);
    }

    bool operator <(const emkb2qt &that) const noexcept
    {
         return ::strcmp(em, that.em) < 0;
    }
    constexpr int strcmp(const emkb2qt &that, const int i = 0) const
     {
         return em[i] == 0 && that.em[i] == 0 ? 0
             : em[i] == 0 ? -1
                 : that.em[i] == 0 ? 1
                     : em[i] < that.em[i] ? -1
                         : em[i] > that.em[i] ? 1
                             : strcmp(that, i + 1);
     }
} emkb2qt_t;

template<unsigned int Qt, char ... EmChar>
struct Emkb2Qt
{
    static constexpr const char storage[sizeof ... (EmChar) + 1] = {EmChar..., '\0'};
    using Type = emkb2qt_t;
    static constexpr Type data() noexcept { return Type{storage, Qt}; }
};

template<unsigned int Qt, char ... EmChar> constexpr char Emkb2Qt<Qt, EmChar...>::storage[];

static constexpr const auto KeyTbl = qMakeArray(
    QSortedData<
        Emkb2Qt< Qt::Key_Escape,        'E','s','c','a','p','e' >,
        Emkb2Qt< Qt::Key_Tab,           'T','a','b' >,
        Emkb2Qt< Qt::Key_Backspace,     'B','a','c','k','s','p','a','c','e' >,
        Emkb2Qt< Qt::Key_Return,        'E','n','t','e','r' >,
        Emkb2Qt< Qt::Key_Insert,        'I','n','s','e','r','t' >,
        Emkb2Qt< Qt::Key_Delete,        'D','e','l','e','t','e' >,
        Emkb2Qt< Qt::Key_Pause,         'P','a','u','s','e' >,
        Emkb2Qt< Qt::Key_Pause,         'C','l','e','a','r' >,
        Emkb2Qt< Qt::Key_Home,          'H','o','m','e' >,
        Emkb2Qt< Qt::Key_End,           'E','n','d' >,
        Emkb2Qt< Qt::Key_Left,          'A','r','r','o','w','L','e','f','t' >,
        Emkb2Qt< Qt::Key_Up,            'A','r','r','o','w','U','p' >,
        Emkb2Qt< Qt::Key_Right,         'A','r','r','o','w','R','i','g','h','t' >,
        Emkb2Qt< Qt::Key_Down,          'A','r','r','o','w','D','o','w','n' >,
        Emkb2Qt< Qt::Key_PageUp,        'P','a','g','e','U','p' >,
        Emkb2Qt< Qt::Key_PageDown,      'P','a','g','e','D','o','w','n' >,
        Emkb2Qt< Qt::Key_Shift,         'S','h','i','f','t' >,
        Emkb2Qt< Qt::Key_Control,       'C','o','n','t','r','o','l' >,
        Emkb2Qt< Qt::Key_Meta,          'O','S'>,
        Emkb2Qt< Qt::Key_Alt,           'A','l','t','L','e','f','t' >,
        Emkb2Qt< Qt::Key_Alt,           'A','l','t' >,
        Emkb2Qt< Qt::Key_CapsLock,      'C','a','p','s','L','o','c','k' >,
        Emkb2Qt< Qt::Key_NumLock,       'N','u','m','L','o','c','k' >,
        Emkb2Qt< Qt::Key_ScrollLock,    'S','c','r','o','l','l','L','o','c','k' >,
        Emkb2Qt< Qt::Key_F1,            'F','1' >,
        Emkb2Qt< Qt::Key_F2,            'F','2' >,
        Emkb2Qt< Qt::Key_F3,            'F','3' >,
        Emkb2Qt< Qt::Key_F4,            'F','4' >,
        Emkb2Qt< Qt::Key_F5,            'F','5' >,
        Emkb2Qt< Qt::Key_F6,            'F','6' >,
        Emkb2Qt< Qt::Key_F7,            'F','7' >,
        Emkb2Qt< Qt::Key_F8,            'F','8' >,
        Emkb2Qt< Qt::Key_F9,            'F','9' >,
        Emkb2Qt< Qt::Key_F10,           'F','1','0' >,
        Emkb2Qt< Qt::Key_F11,           'F','1','1' >,
        Emkb2Qt< Qt::Key_F12,           'F','1','2' >,
        Emkb2Qt< Qt::Key_F13,           'F','1','3' >,
        Emkb2Qt< Qt::Key_F14,           'F','1','4' >,
        Emkb2Qt< Qt::Key_F15,           'F','1','5' >,
        Emkb2Qt< Qt::Key_F16,           'F','1','6' >,
        Emkb2Qt< Qt::Key_F17,           'F','1','7' >,
        Emkb2Qt< Qt::Key_F18,           'F','1','8' >,
        Emkb2Qt< Qt::Key_F19,           'F','1','9' >,
        Emkb2Qt< Qt::Key_F20,           'F','2','0' >,
        Emkb2Qt< Qt::Key_F21,           'F','2','1' >,
        Emkb2Qt< Qt::Key_F22,           'F','2','2' >,
        Emkb2Qt< Qt::Key_F23,           'F','2','3' >,
        Emkb2Qt< Qt::Key_Space,         ' ' >,
        Emkb2Qt< Qt::Key_Comma,         ',' >,
        Emkb2Qt< Qt::Key_Minus,         '-' >,
        Emkb2Qt< Qt::Key_Period,        '.' >,
        Emkb2Qt< Qt::Key_Slash,         '/' >,
        Emkb2Qt< Qt::Key_0,             'D','i','g','i','t','0' >,
        Emkb2Qt< Qt::Key_1,             'D','i','g','i','t','1' >,
        Emkb2Qt< Qt::Key_2,             'D','i','g','i','t','2' >,
        Emkb2Qt< Qt::Key_3,             'D','i','g','i','t','3' >,
        Emkb2Qt< Qt::Key_4,             'D','i','g','i','t','4' >,
        Emkb2Qt< Qt::Key_5,             'D','i','g','i','t','5' >,
        Emkb2Qt< Qt::Key_6,             'D','i','g','i','t','6' >,
        Emkb2Qt< Qt::Key_7,             'D','i','g','i','t','7' >,
        Emkb2Qt< Qt::Key_8,             'D','i','g','i','t','8' >,
        Emkb2Qt< Qt::Key_9,             'D','i','g','i','t','9' >,
        Emkb2Qt< Qt::Key_Semicolon,     ';' >,
        Emkb2Qt< Qt::Key_Equal,         '=' >,
        Emkb2Qt< Qt::Key_A,             'K','e','y','A' >,
        Emkb2Qt< Qt::Key_B,             'K','e','y','B' >,
        Emkb2Qt< Qt::Key_C,             'K','e','y','C' >,
        Emkb2Qt< Qt::Key_D,             'K','e','y','D' >,
        Emkb2Qt< Qt::Key_E,             'K','e','y','E' >,
        Emkb2Qt< Qt::Key_F,             'K','e','y','F' >,
        Emkb2Qt< Qt::Key_G,             'K','e','y','G' >,
        Emkb2Qt< Qt::Key_H,             'K','e','y','H' >,
        Emkb2Qt< Qt::Key_I,             'K','e','y','I' >,
        Emkb2Qt< Qt::Key_J,             'K','e','y','J' >,
        Emkb2Qt< Qt::Key_K,             'K','e','y','K' >,
        Emkb2Qt< Qt::Key_L,             'K','e','y','L' >,
        Emkb2Qt< Qt::Key_M,             'K','e','y','M' >,
        Emkb2Qt< Qt::Key_N,             'K','e','y','N' >,
        Emkb2Qt< Qt::Key_O,             'K','e','y','O' >,
        Emkb2Qt< Qt::Key_P,             'K','e','y','P' >,
        Emkb2Qt< Qt::Key_Q,             'K','e','y','Q' >,
        Emkb2Qt< Qt::Key_R,             'K','e','y','R' >,
        Emkb2Qt< Qt::Key_S,             'K','e','y','S' >,
        Emkb2Qt< Qt::Key_T,             'K','e','y','T' >,
        Emkb2Qt< Qt::Key_U,             'K','e','y','U' >,
        Emkb2Qt< Qt::Key_V,             'K','e','y','V' >,
        Emkb2Qt< Qt::Key_W,             'K','e','y','W' >,
        Emkb2Qt< Qt::Key_X,             'K','e','y','X' >,
        Emkb2Qt< Qt::Key_Y,             'K','e','y','Y' >,
        Emkb2Qt< Qt::Key_Z,             'K','e','y','Z' >,
        Emkb2Qt< Qt::Key_BracketLeft,   '[' >,
        Emkb2Qt< Qt::Key_Backslash,     '\\' >,
        Emkb2Qt< Qt::Key_BracketRight,  ']' >,
        Emkb2Qt< Qt::Key_Apostrophe,    '\'' >,
        Emkb2Qt< Qt::Key_QuoteLeft,     'B','a','c','k','q','u','o','t','e' >,
        Emkb2Qt< Qt::Key_multiply,      'N','u','m','p','a','d','M','u','l','t','i','p','l','y' >,
        Emkb2Qt< Qt::Key_Minus,         'N','u','m','p','a','d','S','u','b','t','r','a','c','t' >,
        Emkb2Qt< Qt::Key_Period,        'N','u','m','p','a','d','D','e','c','i','m','a','l' >,
        Emkb2Qt< Qt::Key_Plus,          'N','u','m','p','a','d','A','d','d' >,
        Emkb2Qt< Qt::Key_division,      'N','u','m','p','a','d','D','i','v','i','d','e' >,
        Emkb2Qt< Qt::Key_Equal,         'N','u','m','p','a','d','E','q','u','a','l' >,
        Emkb2Qt< Qt::Key_0,             'N','u','m','p','a','d','0' >,
        Emkb2Qt< Qt::Key_1,             'N','u','m','p','a','d','1' >,
        Emkb2Qt< Qt::Key_2,             'N','u','m','p','a','d','2' >,
        Emkb2Qt< Qt::Key_3,             'N','u','m','p','a','d','3' >,
        Emkb2Qt< Qt::Key_4,             'N','u','m','p','a','d','4' >,
        Emkb2Qt< Qt::Key_5,             'N','u','m','p','a','d','5' >,
        Emkb2Qt< Qt::Key_6,             'N','u','m','p','a','d','6' >,
        Emkb2Qt< Qt::Key_7,             'N','u','m','p','a','d','7' >,
        Emkb2Qt< Qt::Key_8,             'N','u','m','p','a','d','8' >,
        Emkb2Qt< Qt::Key_9,             'N','u','m','p','a','d','9' >,
        Emkb2Qt< Qt::Key_Comma,         'N','u','m','p','a','d','C','o','m','m','a' >,
        Emkb2Qt< Qt::Key_Enter,         'N','u','m','p','a','d','E','n','t','e','r' >,
        Emkb2Qt< Qt::Key_Paste,         'P','a','s','t','e' >,
        Emkb2Qt< Qt::Key_AltGr,         'A','l','t','R','i','g','h','t' >,
        Emkb2Qt< Qt::Key_Help,          'H','e','l','p' >,
        Emkb2Qt< Qt::Key_Equal,         '=' >,
        Emkb2Qt< Qt::Key_yen,           'I','n','t','l','Y','e','n' >,

        Emkb2Qt< Qt::Key_Exclam,        '\x21' >,
        Emkb2Qt< Qt::Key_QuoteDbl,      '\x22' >,
        Emkb2Qt< Qt::Key_NumberSign,    '\x23' >,
        Emkb2Qt< Qt::Key_Dollar,        '\x24' >,
        Emkb2Qt< Qt::Key_Percent,       '\x25' >,
        Emkb2Qt< Qt::Key_Ampersand,     '\x26' >,
        Emkb2Qt< Qt::Key_ParenLeft,     '\x28' >,
        Emkb2Qt< Qt::Key_ParenRight,    '\x29' >,
        Emkb2Qt< Qt::Key_Asterisk,      '\x2a' >,
        Emkb2Qt< Qt::Key_Plus,          '\x2b' >,
        Emkb2Qt< Qt::Key_Colon,         '\x3a' >,
        Emkb2Qt< Qt::Key_Semicolon,     '\x3b' >,
        Emkb2Qt< Qt::Key_Less,          '\x3c' >,
        Emkb2Qt< Qt::Key_Equal,         '\x3d' >,
        Emkb2Qt< Qt::Key_Greater,       '\x3e' >,
        Emkb2Qt< Qt::Key_Question,      '\x3f' >,
        Emkb2Qt< Qt::Key_At,            '\x40' >,
        Emkb2Qt< Qt::Key_BracketLeft,   '\x5b' >,
        Emkb2Qt< Qt::Key_Backslash,     '\x5c' >,
        Emkb2Qt< Qt::Key_BracketRight,  '\x5d' >,
        Emkb2Qt< Qt::Key_AsciiCircum,   '\x5e' >,
        Emkb2Qt< Qt::Key_Underscore,    '\x5f' >,
        Emkb2Qt< Qt::Key_QuoteLeft,     '\x60'>,
        Emkb2Qt< Qt::Key_BraceLeft,     '\x7b'>,
        Emkb2Qt< Qt::Key_Bar,           '\x7c'>,
        Emkb2Qt< Qt::Key_BraceRight,    '\x7d'>,
        Emkb2Qt< Qt::Key_AsciiTilde,    '\x7e'>,
        Emkb2Qt< Qt::Key_Space,         '\x20' >,
        Emkb2Qt< Qt::Key_Comma,         '\x2c' >,
        Emkb2Qt< Qt::Key_Minus,         '\x2d' >,
        Emkb2Qt< Qt::Key_Period,        '\x2e' >,
        Emkb2Qt< Qt::Key_Slash,         '\x2f' >,
        Emkb2Qt< Qt::Key_Apostrophe,    '\x27' >,
        Emkb2Qt< Qt::Key_Menu,          'C','o','n','t','e','x','t','M','e','n','u' >,

        Emkb2Qt< Qt::Key_Agrave,        '\xc3','\xa0' >,
        Emkb2Qt< Qt::Key_Aacute,        '\xc3','\xa1' >,
        Emkb2Qt< Qt::Key_Acircumflex,   '\xc3','\xa2' >,
        Emkb2Qt< Qt::Key_Adiaeresis,    '\xc3','\xa4' >,
        Emkb2Qt< Qt::Key_AE,            '\xc3','\xa6' >,
        Emkb2Qt< Qt::Key_Atilde,        '\xc3','\xa3' >,
        Emkb2Qt< Qt::Key_Aring,         '\xc3','\xa5' >,
        Emkb2Qt< Qt::Key_Ccedilla,      '\xc3','\xa7' >,
        Emkb2Qt< Qt::Key_Egrave,        '\xc3','\xa8' >,
        Emkb2Qt< Qt::Key_Eacute,        '\xc3','\xa9' >,
        Emkb2Qt< Qt::Key_Ecircumflex,   '\xc3','\xaa' >,
        Emkb2Qt< Qt::Key_Ediaeresis,    '\xc3','\xab' >,
        Emkb2Qt< Qt::Key_Icircumflex,   '\xc3','\xae' >,
        Emkb2Qt< Qt::Key_Idiaeresis,    '\xc3','\xaf' >,
        Emkb2Qt< Qt::Key_Ocircumflex,   '\xc3','\xb4' >,
        Emkb2Qt< Qt::Key_Odiaeresis,    '\xc3','\xb6' >,
        Emkb2Qt< Qt::Key_Ograve,        '\xc3','\xb2' >,
        Emkb2Qt< Qt::Key_Oacute,        '\xc3','\xb3' >,
        Emkb2Qt< Qt::Key_Ooblique,      '\xc3','\xb8' >,
        Emkb2Qt< Qt::Key_Otilde,        '\xc3','\xb5' >,
        Emkb2Qt< Qt::Key_Ucircumflex,   '\xc3','\xbb' >,
        Emkb2Qt< Qt::Key_Udiaeresis,    '\xc3','\xbc' >,
        Emkb2Qt< Qt::Key_Ugrave,        '\xc3','\xb9' >,
        Emkb2Qt< Qt::Key_Uacute,        '\xc3','\xba' >,
        Emkb2Qt< Qt::Key_Ntilde,        '\xc3','\xb1' >,
        Emkb2Qt< Qt::Key_ydiaeresis,    '\xc3','\xbf' >
            >::Data{}
        );

static constexpr const auto DeadKeyShiftTbl = qMakeArray(
    QSortedData<
       // shifted
        Emkb2Qt< Qt::Key_Agrave,        '\xc3','\x80' >,
        Emkb2Qt< Qt::Key_Aacute,        '\xc3','\x81' >,
        Emkb2Qt< Qt::Key_Acircumflex,   '\xc3','\x82' >,
        Emkb2Qt< Qt::Key_Adiaeresis,    '\xc3','\x84' >,
        Emkb2Qt< Qt::Key_AE,            '\xc3','\x86' >,
        Emkb2Qt< Qt::Key_Atilde,        '\xc3','\x83' >,
        Emkb2Qt< Qt::Key_Aring,         '\xc3','\x85' >,
        Emkb2Qt< Qt::Key_Egrave,        '\xc3','\x88' >,
        Emkb2Qt< Qt::Key_Eacute,        '\xc3','\x89' >,
        Emkb2Qt< Qt::Key_Ecircumflex,   '\xc3','\x8a' >,
        Emkb2Qt< Qt::Key_Ediaeresis,    '\xc3','\x8b' >,
        Emkb2Qt< Qt::Key_Icircumflex,   '\xc3','\x8e' >,
        Emkb2Qt< Qt::Key_Idiaeresis,    '\xc3','\x8f' >,
        Emkb2Qt< Qt::Key_Ocircumflex,   '\xc3','\x94' >,
        Emkb2Qt< Qt::Key_Odiaeresis,    '\xc3','\x96' >,
        Emkb2Qt< Qt::Key_Ograve,        '\xc3','\x92' >,
        Emkb2Qt< Qt::Key_Oacute,        '\xc3','\x93' >,
        Emkb2Qt< Qt::Key_Ooblique,      '\xc3','\x98' >,
        Emkb2Qt< Qt::Key_Otilde,        '\xc3','\x95' >,
        Emkb2Qt< Qt::Key_Ucircumflex,   '\xc3','\x9b' >,
        Emkb2Qt< Qt::Key_Udiaeresis,    '\xc3','\x9c' >,
        Emkb2Qt< Qt::Key_Ugrave,        '\xc3','\x99' >,
        Emkb2Qt< Qt::Key_Uacute,        '\xc3','\x9a' >,
        Emkb2Qt< Qt::Key_Ntilde,        '\xc3','\x91' >,
        Emkb2Qt< Qt::Key_Ccedilla,      '\xc3','\x87' >,
        Emkb2Qt< Qt::Key_ydiaeresis,    '\xc3','\x8f' >
    >::Data{}
);

// macOS CTRL <-> META switching. We most likely want to enable
// the existing switching code in QtGui, but for now do it here.
static bool g_usePlatformMacCtrlMetaSwitching = false;

bool g_useNaturalScrolling = true; // natural scrolling is default on linux/windows

static void mouseWheelEvent(emscripten::val event) {

    emscripten::val wheelInterted = event["webkitDirectionInvertedFromDevice"];

    g_useNaturalScrolling = !wheelInterted.as<bool>();
}

static std::set<QWasmEventTranslator *> s_eventTranslators;

EMSCRIPTEN_BINDINGS(qtMouseModule) {
    function("qtMouseWheelEvent", &mouseWheelEvent);
}

QMap<int, QPointF> QWasmEventTranslator::pressedTouchIds;
QWasmEventTranslator *QWasmEventTranslator::capturedTranslator = nullptr;
QWasmEventTranslator *QWasmEventTranslator::mouseInTranslator = nullptr;
QPointer<QWindow> QWasmEventTranslator::mouseInWindow;
QPointer<QWindow> QWasmEventTranslator::lastMouseInWindow;
QPointer<QWindow> QWasmEventTranslator::draggedWindow;
QWindow *QWasmEventTranslator::capturedWindow = nullptr;
Qt::MouseButtons QWasmEventTranslator::pressedButtons(Qt::NoButton);

QWasmWindow::ResizeMode QWasmEventTranslator::resizeMode = QWasmWindow::ResizeNone;
QPoint QWasmEventTranslator::resizePoint;
QPoint QWasmEventTranslator::lastMousePoint;
QRect QWasmEventTranslator::resizeStartRect;
QTouchDevice *QWasmEventTranslator::touchDevice = nullptr;

Qt::Key QWasmEventTranslator::m_emDeadKey = Qt::Key_unknown;
bool QWasmEventTranslator::m_emStickyDeadKey = false;

QWasmEventTranslator::QWasmEventTranslator(QWasmScreen *screen)
    : QObject(screen)
{
    s_eventTranslators.insert(this);
    if (!touchDevice)
    {
        touchDevice = new QTouchDevice;
        touchDevice->setType(QTouchDevice::TouchScreen);
        touchDevice->setCapabilities(QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::NormalizedPosition);
        QWindowSystemInterface::registerTouchDevice(touchDevice);
    }

    initEventHandlers();
}

QWasmEventTranslator::~QWasmEventTranslator()
{
    s_eventTranslators.erase(this);
    if (s_eventTranslators.empty())
    {
        QWindowSystemInterface::unregisterTouchDevice(touchDevice);
        delete touchDevice;
        touchDevice = nullptr;
    }

    if (mouseInTranslator == this)
    {
        mouseInTranslator = nullptr;
    }

    if (capturedTranslator == this)
    {
        pressedTouchIds.clear();
        capturedTranslator = nullptr;
        capturedWindow = nullptr;
        draggedWindow.clear();
        mouseInWindow.clear();
        lastMouseInWindow.clear();
        pressedButtons = Qt::NoButton;
        resizeMode = QWasmWindow::ResizeNone;
    }
}

void QWasmEventTranslator::initEventHandlers()
{
    QByteArray _canvasId = screen()->canvasId().toUtf8();
    const char *canvasId = _canvasId.constData();

    // The Platform Detect: expand coverage and move as needed
    enum Platform {
        GenericPlatform,
        MacOSPlatform
    };
    Platform platform = Platform(emscripten::val::global("navigator")["platform"]
            .call<bool>("includes", emscripten::val("Mac")));
    g_usePlatformMacCtrlMetaSwitching = (platform == MacOSPlatform);

    if (platform == MacOSPlatform) {
        g_useNaturalScrolling = false; // make this !default on macOS

        if (emscripten::val::global("window")["safari"].isUndefined()) {
            val document = val::global("document");
            val jsCanvasId = QWasmString::fromQString(screen()->canvasId());
            val canvas = document.call<val>("getElementById", jsCanvasId);
            canvas.call<void>("addEventListener",
                              val("wheel"),
                              val::module_property("qtMouseWheelEvent"));
        }
    }

    emscripten_set_mousedown_callback(canvasId, (void *)this, 0, &mouse_cb);
    emscripten_set_mouseenter_callback(canvasId, (void *)this, 0, &mouse_cb);
    emscripten_set_mouseleave_callback(canvasId, (void *)this, 0, &mouse_cb);

    emscripten_set_focus_callback(canvasId, (void *)this, 0, &focus_cb);
    emscripten_set_focusout_callback(canvasId, (void *)this, 0, &focus_cb);

    emscripten_set_wheel_callback(canvasId, (void *)this, 0, &wheel_cb);

    emscripten_set_touchstart_callback(canvasId, (void *)this, 0, &touchCallback);

    static bool initOnce = false;
    if (!initOnce)
    {
        initOnce = true;
        emscripten_set_keydown_callback("#document", NULL, 0, &keyboard_cb);
        emscripten_set_keyup_callback("#document", NULL, 0, &keyboard_cb);

        emscripten_set_mouseup_callback("#document", NULL, 0, &mouse_cb);
        emscripten_set_mousemove_callback("#document", NULL, 0, &mouse_cb);

        emscripten_set_touchend_callback("#document", NULL, 0, &touchCallback);
        emscripten_set_touchmove_callback("#document", NULL, 0, &touchCallback);
        emscripten_set_touchcancel_callback("#document", NULL, 0, &touchCallback);
    }
}

template <typename Event>
QFlags<Qt::KeyboardModifier> QWasmEventTranslator::translatKeyModifier(const Event *event)
{
    QFlags<Qt::KeyboardModifier> keyModifier = Qt::NoModifier;
    if (event->shiftKey)
        keyModifier |= Qt::ShiftModifier;
    if (event->ctrlKey) {
        if (g_usePlatformMacCtrlMetaSwitching)
            keyModifier |= Qt::MetaModifier;
        else
            keyModifier |= Qt::ControlModifier;
    }
    if (event->altKey)
        keyModifier |= Qt::AltModifier;
    if (event->metaKey) {
        if (g_usePlatformMacCtrlMetaSwitching)
            keyModifier |= Qt::ControlModifier;
        else
            keyModifier |= Qt::MetaModifier;
    }
    return keyModifier;
}

QFlags<Qt::KeyboardModifier> QWasmEventTranslator::translateKeyboardEventModifier(const EmscriptenKeyboardEvent *keyEvent)
{
    QFlags<Qt::KeyboardModifier> keyModifier = translatKeyModifier(keyEvent);
    if (keyEvent->location == DOM_KEY_LOCATION_NUMPAD) {
        keyModifier |= Qt::KeypadModifier;
    }

    return keyModifier;
}

QFlags<Qt::KeyboardModifier> QWasmEventTranslator::translateMouseEventModifier(const EmscriptenMouseEvent *mouseEvent)
{
    return translatKeyModifier(mouseEvent);
}

int QWasmEventTranslator::keyboard_cb(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *)
{
    if (!QGuiApplication::focusWindow())
        return 0;
    int accepted = processKeyboard(eventType, keyEvent) ? 1 : 0;
    return accepted;
}

QWasmScreen *QWasmEventTranslator::screen()
{
    return static_cast<QWasmScreen *>(parent());
}

Qt::Key QWasmEventTranslator::translateEmscriptKey(const EmscriptenKeyboardEvent *emscriptKey)
{
    Qt::Key qtKey = Qt::Key_unknown;

    if (qstrncmp(emscriptKey->code, "Key", 3) == 0 || qstrncmp(emscriptKey->code, "Numpad", 6) == 0 ||
        qstrncmp(emscriptKey->code, "Digit", 5) == 0) {

        emkb2qt_t searchKey{emscriptKey->code, 0}; // search emcsripten code
        auto it1 = std::lower_bound(KeyTbl.cbegin(), KeyTbl.cend(), searchKey);
        if (it1 != KeyTbl.end() && !(searchKey < *it1)) {
            qtKey = static_cast<Qt::Key>(it1->qt);
        }
    } else if (qstrncmp(emscriptKey->key, "Dead", 4) == 0 ) {
        emkb2qt_t searchKey1{emscriptKey->code, 0};
        for (auto it1 = KeyTbl.cbegin(); it1 != KeyTbl.end(); ++it1)
            if (it1 != KeyTbl.end() && (qstrcmp(searchKey1.em, it1->em) == 0)) {
                qtKey = static_cast<Qt::Key>(it1->qt);
            }
    }

    if (qtKey == Qt::Key_unknown) {
        emkb2qt_t searchKey{emscriptKey->key, 0}; // search unicode key
        auto it1 = std::lower_bound(KeyTbl.cbegin(), KeyTbl.cend(), searchKey);
        if (it1 != KeyTbl.end() && !(searchKey < *it1)) {
            qtKey = static_cast<Qt::Key>(it1->qt);
        }
    }
    if (qtKey == Qt::Key_unknown) {//try harder with shifted number keys
        emkb2qt_t searchKey1{emscriptKey->key, 0};
        for (auto it1 = KeyTbl.cbegin(); it1 != KeyTbl.end(); ++it1)
            if (it1 != KeyTbl.end() && (qstrcmp(searchKey1.em, it1->em) == 0)) {
                qtKey = static_cast<Qt::Key>(it1->qt);
            }
    }

    return qtKey;
}

Qt::MouseButton QWasmEventTranslator::translateMouseButton(unsigned short button)
{
    if (button == 0)
        return Qt::LeftButton;
    else if (button == 1)
        return Qt::MiddleButton;
    else if (button == 2)
        return Qt::RightButton;

    return Qt::NoButton;
}

int QWasmEventTranslator::mouse_cb(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    bool accepted = false;
    auto translator = reinterpret_cast<QWasmEventTranslator *>(userData);

    switch (eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
        accepted = translator->processMouse(eventType, mouseEvent);
        break;
    case EMSCRIPTEN_EVENT_MOUSEENTER:
        accepted = translator->processMouseEnter(mouseEvent);
        break;
    case EMSCRIPTEN_EVENT_MOUSELEAVE:
        accepted = translator->processMouseLeave();
        break;
    default:
        if (capturedTranslator) {
            translator = capturedTranslator;
        } else {
            translator = mouseInWindow ? mouseInTranslator : nullptr;
        }
        if (translator) {
            accepted = translator->processMouse(eventType, mouseEvent);
        }
    }

    QWasmEventDispatcher::maintainTimers();
    return 0;
}

void resizeWindow(QWindow *window, QWasmWindow::ResizeMode mode,
                  QRect startRect, QPoint amount)
{
    if (mode == QWasmWindow::ResizeNone)
        return;

    bool top = mode == QWasmWindow::ResizeTopLeft ||
               mode == QWasmWindow::ResizeTop ||
               mode == QWasmWindow::ResizeTopRight;

    bool bottom = mode == QWasmWindow::ResizeBottomLeft ||
                  mode == QWasmWindow::ResizeBottom ||
                  mode == QWasmWindow::ResizeBottomRight;

    bool left = mode == QWasmWindow::ResizeLeft ||
                mode == QWasmWindow::ResizeTopLeft ||
                mode == QWasmWindow::ResizeBottomLeft;

    bool right = mode == QWasmWindow::ResizeRight ||
                 mode == QWasmWindow::ResizeTopRight ||
                 mode == QWasmWindow::ResizeBottomRight;

    int x1 = startRect.left();
    int y1 = startRect.top();
    int x2 = startRect.right();
    int y2 = startRect.bottom();

    if (left)
        x1 += amount.x();
    if (top)
        y1 += amount.y();
    if (right)
        x2 += amount.x();
    if (bottom)
        y2 += amount.y();

    int w = x2-x1;
    int h = y2-y1;

    if (w < window->minimumWidth()) {
        if (left)
            x1 -= window->minimumWidth() - w;

        w = window->minimumWidth();
    }

    if (h < window->minimumHeight()) {
        if (top)
            y1 -= window->minimumHeight() - h;

        h = window->minimumHeight();
    }

    window->setGeometry(x1, y1, w, h);
}

void QWasmEventTranslator::leaveWindow(QWindow *window)
{
    QWindowSystemInterface::handleLeaveEvent<QWindowSystemInterface::SynchronousDelivery>(window);
}

void QWasmEventTranslator::enterWindow(QWindow *window, const QPoint &localPoint, const QPoint &globalPoint)
{
    QWindowSystemInterface::handleEnterEvent<QWindowSystemInterface::SynchronousDelivery>(window, localPoint, globalPoint);
}

bool QWasmEventTranslator::processMouseEnter(const EmscriptenMouseEvent *mouseEvent)
{
    QPoint globalPoint(mouseEvent->clientX, mouseEvent->clientY);

    mouseInTranslator = this;

    auto window = screen()->compositor()->windowAt(globalPoint, 0);
    if (mouseInWindow == window)
        return true;

    QPoint localPoint;
    if (window) {
        localPoint = window->mapFromGlobal(globalPoint);

        if (!QRect(QPoint(), window->size()).contains(localPoint))
            window = nullptr;
    }

    if (mouseInWindow && !capturedWindow) {
        leaveWindow(mouseInWindow.data());
    }

    mouseInWindow = window;

    if (!capturedWindow && window) {
        enterWindow(mouseInWindow.data(), localPoint, globalPoint);
    }
    return true;
}

bool QWasmEventTranslator::processMouseLeave()
{
    if (mouseInTranslator == this) {
        mouseInTranslator = nullptr;
    }

    if (mouseInWindow && mouseInWindow->screen() != screen()->screen())
        return true;

    auto window = mouseInWindow.data();
    if (!window)
        return true;

    mouseInWindow.clear();
    if (!capturedWindow) {
        leaveWindow(window);
    }
    return true;
}

bool QWasmEventTranslator::ensureWindowCaptured(QWindow *window)
{
    if (!capturedWindow)
    {
        QWindow *modalWindow;
        bool blocked = QGuiApplicationPrivate::instance()->isWindowBlocked(window, &modalWindow);
        if (blocked)
            return false;

        lastMouseInWindow = mouseInWindow;
        capturedWindow = window;
        capturedTranslator = this;

        window->requestActivate();
        if (window->windowState() == Qt::WindowNoState)
            window->raise();
    }

    return true;
}

void QWasmEventTranslator::maybeReleaseCapturedWindow()
{
    if (capturedWindow && pressedTouchIds.isEmpty() && !pressedButtons) {
        releaseCapturedWindow();
    }
}

void QWasmEventTranslator::releaseIfCapturedWindow(QWasmWindow* window)
{
    if (window && capturedWindow == window->window()) {
        releaseCapturedWindow();
    }
}

void QWasmEventTranslator::releaseCapturedWindow()
{
    if (!capturedWindow)
        return;

    auto windowToLeave = lastMouseInWindow.data();
    lastMouseInWindow.clear();
    draggedWindow.clear();
    capturedWindow = nullptr;
    capturedTranslator = nullptr;
    if (windowToLeave != mouseInWindow.data()) {
        if (windowToLeave) {
            leaveWindow(windowToLeave);
        }
        if (mouseInWindow) {
            QPoint localPoint = mouseInWindow->mapFromGlobal(lastMousePoint);
            enterWindow(mouseInWindow.data(), localPoint, lastMousePoint);
        }
    }
}

bool QWasmEventTranslator::processMouse(int eventType, const EmscriptenMouseEvent *mouseEvent)
{
    auto timestamp = mouseEvent->timestamp;
    QPoint globalPoint(mouseEvent->clientX, mouseEvent->clientY);
    lastMousePoint = globalPoint;

    QEvent::Type buttonEventType = QEvent::None;
    Qt::MouseButton button = translateMouseButton(mouseEvent->button);
    Qt::KeyboardModifiers modifiers = translateMouseEventModifier(mouseEvent);

    QWindow *window = screen()->compositor()->windowAt(globalPoint, 0);
    QWindow *window2 = capturedWindow ? capturedWindow : window;

    QPoint localPoint = window2 ? window2->mapFromGlobal(globalPoint) : QPoint();

    QWasmWindow *htmlWindow = window2
        ? static_cast<QWasmWindow*>(window2->handle())
        : nullptr;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
    {
        if (!htmlWindow)
            return false;

        if (!ensureWindowCaptured(window2))
            return false;

        pressedButtons.setFlag(button);

        if (button == Qt::LeftButton) {
            if (htmlWindow->isPointOnTitle(localPoint)) {
                draggedWindow = window2;
                resizePoint = globalPoint - window2->mapToGlobal(QPoint(0, 0));
            } else if (htmlWindow->isPointOnResizeRegion(localPoint)) {
                draggedWindow = window2;
                resizeMode = htmlWindow->resizeModeAtPoint(localPoint);
                resizePoint = globalPoint;
                resizeStartRect = window2->geometry();
            }
        }
        htmlWindow->injectMousePressed(localPoint, button, modifiers);

        if (draggedWindow)
            return true;
        buttonEventType = QEvent::MouseButtonPress;
        break;
    }
    case EMSCRIPTEN_EVENT_MOUSEUP:
    {
        if (!capturedWindow)
            return false;

        if (!pressedButtons.testFlag(button))
            return true;

        pressedButtons.setFlag(button, false);

        bool wasDragged = !!draggedWindow;

        if (draggedWindow && button == Qt::LeftButton) {
            draggedWindow.clear();
            resizeMode = QWasmWindow::ResizeNone;
        }

        if (capturedWindow) {
            capturedWindow->requestActivate();
        }

        htmlWindow->injectMouseReleased(localPoint, button, modifiers);
        maybeReleaseCapturedWindow();

        if (wasDragged)
            return true;

        buttonEventType = QEvent::MouseButtonRelease;
        break;
    }
    case EMSCRIPTEN_EVENT_MOUSEMOVE: // drag event
    {
        if (draggedWindow || !window || !QRect(QPoint(), window->size()).contains(localPoint)) {
            window = nullptr;
        }

        if (!capturedWindow && mouseInWindow != window) {
            if (mouseInWindow)
                leaveWindow(mouseInWindow.data());

            if (window)
                enterWindow(window, localPoint, globalPoint);
        }

        mouseInWindow = window;

        if (draggedWindow) {
            QPoint delta = globalPoint - resizePoint;
            if (resizeMode == QWasmWindow::ResizeNone) {
                draggedWindow->setPosition(draggedWindow->mapFromGlobal(delta) + draggedWindow->position());
            } else {
                resizeWindow(draggedWindow, resizeMode, resizeStartRect, delta);
            }
            return true;
        }

        buttonEventType = QEvent::MouseMove;
        break;
    }
    default: // MOUSELEAVE MOUSEENTER
        break;
    };

    if (!window2)
        return false;

    QWindowSystemInterface::handleMouseEvent<QWindowSystemInterface::SynchronousDelivery>(
        window2, timestamp, localPoint, globalPoint, pressedButtons, button, buttonEventType, modifiers);
    return true;
}

int QWasmEventTranslator::focus_cb(int eventType, const EmscriptenFocusEvent */*focusEvent*/, void *userData)
{
    switch (eventType) {
    case EMSCRIPTEN_EVENT_FOCUSOUT: {
        if (capturedWindow)
            return 1;

        auto window = QGuiApplication::focusWindow();
        auto translator = reinterpret_cast<QWasmEventTranslator *>(userData);
        if (window && window->screen() == translator->screen()->screen()) {
            QWindowSystemInterface::handleWindowActivated(nullptr);
        }
        return 1;
    }
    }

    return 0;
}

int QWasmEventTranslator::wheel_cb(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    Q_UNUSED(eventType)

    QWasmEventTranslator *eventTranslator = static_cast<QWasmEventTranslator *>(userData);
    EmscriptenMouseEvent mouseEvent = wheelEvent->mouse;

    int scrollFactor = 0;
    switch (wheelEvent->deltaMode) {
    case DOM_DELTA_PIXEL://chrome safari
        scrollFactor = 1;
        break;
    case DOM_DELTA_LINE: //firefox
        scrollFactor = 12;
        break;
    case DOM_DELTA_PAGE:
        scrollFactor = 20;
        break;
    };

    if (g_useNaturalScrolling) //macOS platform has document oriented scrolling
        scrollFactor = -scrollFactor;

    QWasmEventTranslator *translator = (QWasmEventTranslator*)userData;
    Qt::KeyboardModifiers modifiers = translator->translateMouseEventModifier(&mouseEvent);
    auto timestamp = mouseEvent.timestamp;
    QPoint globalPoint(mouseEvent.clientX, mouseEvent.clientY);

    QWindow *window2 = eventTranslator->screen()->compositor()->windowAt(globalPoint, 0);
    if (!window2)
        return 0;
    QPoint localPoint = window2->mapFromGlobal(globalPoint);

    QPoint pixelDelta;

    if (wheelEvent->deltaY != 0) pixelDelta.setY(wheelEvent->deltaY * scrollFactor);
    if (wheelEvent->deltaX != 0) pixelDelta.setX(wheelEvent->deltaX * scrollFactor);

    QWindowSystemInterface::handleWheelEvent(window2, timestamp, localPoint,
                                             globalPoint, QPoint(), pixelDelta, modifiers);
    QWasmEventDispatcher::maintainTimers();

    return 1;
}

int QWasmEventTranslator::touchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    bool accepted = false;
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) {
        auto translator = reinterpret_cast<QWasmEventTranslator *>(userData);
        accepted = translator->handleTouch(eventType, touchEvent);
    } else {
        if (capturedTranslator) {
            accepted = capturedTranslator->handleTouch(eventType, touchEvent);
        }
    }

    QWasmEventDispatcher::maintainTimers();
    return accepted ? 1 : 0;
}

bool QWasmEventTranslator::handleTouch(int eventType, const EmscriptenTouchEvent *touchEvent)
{
    QList<QWindowSystemInterface::TouchPoint> touchPointList;
    touchPointList.reserve(touchEvent->numTouches);
    QWindow *window2 = nullptr;

    if (capturedWindow) {
        window2 = capturedWindow;
    } else {
        for (int i = 0; i < touchEvent->numTouches; i++) {
            const EmscriptenTouchPoint *touches = &touchEvent->touches[i];
            QPoint globalPoint(touches->clientX, touches->clientY);
            window2 = this->screen()->compositor()->windowAt(globalPoint, 0);
            if (window2)
                break;
        }
    }

    if (!window2)
        return false;

    for (int i = 0; i < touchEvent->numTouches; i++) {

        const EmscriptenTouchPoint *touches = &touchEvent->touches[i];

        QPoint globalPoint(touches->clientX, touches->clientY);

        QWindowSystemInterface::TouchPoint touchPoint;

        touchPoint.area = QRect(0, 0, 8, 8);
        touchPoint.id = touches->identifier;
        touchPoint.pressure = 1.0;

        touchPoint.area.moveCenter(globalPoint);

        const auto tp = pressedTouchIds.constFind(touchPoint.id);
        if (tp != pressedTouchIds.constEnd())
            touchPoint.normalPosition = tp.value();

        QPointF localPoint = QPointF(window2->mapFromGlobal(globalPoint));
        QPointF normalPosition(localPoint.x() / window2->width(),
                               localPoint.y() / window2->height());

        const bool stationaryTouchPoint = (normalPosition == touchPoint.normalPosition);
        touchPoint.normalPosition = normalPosition;

        switch (eventType) {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
            if (!ensureWindowCaptured(window2))
                break;

            if (tp != pressedTouchIds.constEnd()) {
                touchPoint.state = (stationaryTouchPoint
                                    ? Qt::TouchPointStationary
                                    : Qt::TouchPointMoved);
            } else {
                touchPoint.state = Qt::TouchPointPressed;
            }
            pressedTouchIds.insert(touchPoint.id, touchPoint.normalPosition);

            break;
        case EMSCRIPTEN_EVENT_TOUCHEND:
            if (!capturedWindow || tp == pressedTouchIds.constEnd())
                break;

            touchPoint.state = Qt::TouchPointReleased;
            pressedTouchIds.remove(touchPoint.id);
            maybeReleaseCapturedWindow();
            break;
        case EMSCRIPTEN_EVENT_TOUCHMOVE:
            if (!capturedWindow)
                break;
            touchPoint.state = (stationaryTouchPoint
                                ? Qt::TouchPointStationary
                                : Qt::TouchPointMoved);

            pressedTouchIds.insert(touchPoint.id, touchPoint.normalPosition);
            break;
        default:
            break;
        }

        touchPointList.append(touchPoint);
    }

    if (!capturedWindow)
        return false;

    QFlags<Qt::KeyboardModifier> keyModifier = translatKeyModifier(touchEvent);

    QWindowSystemInterface::handleTouchEvent<QWindowSystemInterface::SynchronousDelivery>(
                window2, getTimestamp(), touchDevice, touchPointList, keyModifier);

    if (eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL)
        QWindowSystemInterface::handleTouchCancelEvent(window2, getTimestamp(), touchDevice, keyModifier);

    return true;
}

quint64 QWasmEventTranslator::getTimestamp()
{
    return QDeadlineTimer::current().deadlineNSecs() / 1000;
}

struct KeyMapping { Qt::Key from, to; };

constexpr KeyMapping tildeKeyTable[] = { // ~
    { Qt::Key_A, Qt::Key_Atilde },
    { Qt::Key_N, Qt::Key_Ntilde },
    { Qt::Key_O, Qt::Key_Otilde },
};
constexpr KeyMapping graveKeyTable[] = { // `
    { Qt::Key_A, Qt::Key_Agrave },
    { Qt::Key_E, Qt::Key_Egrave },
    { Qt::Key_I, Qt::Key_Igrave },
    { Qt::Key_O, Qt::Key_Ograve },
    { Qt::Key_U, Qt::Key_Ugrave },
};
constexpr KeyMapping acuteKeyTable[] = { // '
    { Qt::Key_A, Qt::Key_Aacute },
    { Qt::Key_E, Qt::Key_Eacute },
    { Qt::Key_I, Qt::Key_Iacute },
    { Qt::Key_O, Qt::Key_Oacute },
    { Qt::Key_U, Qt::Key_Uacute },
    { Qt::Key_Y, Qt::Key_Yacute },
};
constexpr KeyMapping diaeresisKeyTable[] = { // umlaut ¨
    { Qt::Key_A, Qt::Key_Adiaeresis },
    { Qt::Key_E, Qt::Key_Ediaeresis },
    { Qt::Key_I, Qt::Key_Idiaeresis },
    { Qt::Key_O, Qt::Key_Odiaeresis },
    { Qt::Key_U, Qt::Key_Udiaeresis },
    { Qt::Key_Y, Qt::Key_ydiaeresis },
};
constexpr KeyMapping circumflexKeyTable[] = { // ^
    { Qt::Key_A, Qt::Key_Acircumflex },
    { Qt::Key_E, Qt::Key_Ecircumflex },
    { Qt::Key_I, Qt::Key_Icircumflex },
    { Qt::Key_O, Qt::Key_Ocircumflex },
    { Qt::Key_U, Qt::Key_Ucircumflex },
};

static Qt::Key find_impl(const KeyMapping *first, const KeyMapping *last, Qt::Key key) noexcept
{
    while (first != last) {
        if (first->from == key)
            return first->to;
        ++first;
    }
    return Qt::Key_unknown;
}

template <size_t N>
static Qt::Key find(const KeyMapping (&map)[N], Qt::Key key) noexcept
{
    return find_impl(map, map + N, key);
}

Qt::Key QWasmEventTranslator::translateDeadKey(Qt::Key deadKey, Qt::Key accentBaseKey)
{
    Qt::Key wasmKey = Qt::Key_unknown;
    switch (deadKey) {
#ifdef Q_OS_MACOS
    case Qt::Key_QuoteLeft: // ` macOS: Key_Dead_Grave
#else
    case Qt::Key_O: // ´ Key_Dead_Grave
#endif
        wasmKey = find(graveKeyTable, accentBaseKey);
        break;
    case Qt::Key_E: // ´ Key_Dead_Acute
        wasmKey = find(acuteKeyTable, accentBaseKey);
        break;
    case Qt::Key_AsciiTilde:
    case Qt::Key_N:// Key_Dead_Tilde
        wasmKey = find(tildeKeyTable, accentBaseKey);
        break;
#ifndef Q_OS_MACOS
    case Qt::Key_QuoteLeft:
#endif
    case Qt::Key_U:// ¨ Key_Dead_Diaeresis
        wasmKey = find(diaeresisKeyTable, accentBaseKey);
        break;
    case Qt::Key_I:// macOS Key_Dead_Circumflex
    case Qt::Key_6:// linux
    case Qt::Key_Apostrophe:// linux
        wasmKey = find(circumflexKeyTable, accentBaseKey);
        break;
    default:
        break;

    };
    return wasmKey;
}

bool QWasmEventTranslator::processKeyboard(int eventType, const EmscriptenKeyboardEvent *keyEvent)
{
    Qt::Key qtKey = translateEmscriptKey(keyEvent);

    Qt::KeyboardModifiers modifiers = translateKeyboardEventModifier(keyEvent);

    QString keyText;
    QEvent::Type keyType = QEvent::None;
    switch (eventType) {
    case EMSCRIPTEN_EVENT_KEYPRESS:
    case EMSCRIPTEN_EVENT_KEYDOWN: // down
        keyType = QEvent::KeyPress;

        if (m_emDeadKey != Qt::Key_unknown) {

            Qt::Key transformedKey = translateDeadKey(m_emDeadKey, qtKey);

            if (transformedKey != Qt::Key_unknown)
                qtKey = transformedKey;

            if (keyEvent->shiftKey == 0) {
                for (auto it = KeyTbl.cbegin(); it != KeyTbl.end(); ++it) {
                    if (it != KeyTbl.end() && (qtKey == static_cast<Qt::Key>(it->qt))) {
                        keyText = it->em;
                        m_emDeadKey = Qt::Key_unknown;
                        break;
                    }
                }
            } else {
                for (auto it = DeadKeyShiftTbl.cbegin(); it != DeadKeyShiftTbl.end(); ++it) {
                    if (it != DeadKeyShiftTbl.end() && (qtKey == static_cast<Qt::Key>(it->qt))) {
                        keyText = it->em;
                        m_emDeadKey = Qt::Key_unknown;
                        break;
                    }
                }
            }
        }
        if (qstrncmp(keyEvent->key, "Dead", 4) == 0 || qtKey == Qt::Key_AltGr) {
            qtKey = translateEmscriptKey(keyEvent);
            m_emStickyDeadKey = true;
            if (keyEvent->shiftKey == 1 && qtKey == Qt::Key_QuoteLeft)
                qtKey = Qt::Key_AsciiTilde;
            m_emDeadKey = qtKey;
        }
        break;
    case EMSCRIPTEN_EVENT_KEYUP: // up
        keyType = QEvent::KeyRelease;
        if (m_emStickyDeadKey && qtKey != Qt::Key_Alt) {
            m_emStickyDeadKey = false;
        }
        break;
    default:
        break;
    };

    if (keyType == QEvent::None)
        return 0;

    QFlags<Qt::KeyboardModifier> mods = translateKeyboardEventModifier(keyEvent);

    // Clipboard fallback path: cut/copy/paste are handled by clipboard event
    // handlers if direct clipboard access is not available.
    if (!QWasmIntegration::get()->getWasmClipboard()->hasClipboardApi && modifiers & Qt::ControlModifier &&
        (qtKey == Qt::Key_X || qtKey == Qt::Key_C || qtKey == Qt::Key_V)) {
            return 0;
    }

    bool accepted = false;

    if (keyType == QEvent::KeyPress &&
            mods.testFlag(Qt::ControlModifier)
            && qtKey == Qt::Key_V) {
        QWasmIntegration::get()->getWasmClipboard()->readTextFromClipboard();
    } else {
        if (keyText.isEmpty())
            keyText = QString(keyEvent->key);
        if (keyText.size() > 1)
            keyText.clear();
        accepted = QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
                    0, keyType, qtKey, modifiers, keyText);
    }
    if (keyType == QEvent::KeyPress &&
            mods.testFlag(Qt::ControlModifier)
            && (qtKey == Qt::Key_C || qtKey == Qt::Key_X)) {
        QWasmIntegration::get()->getWasmClipboard()->writeTextToClipboard();
    }

    QWasmEventDispatcher::maintainTimers();

    return accepted;
}

QT_END_NAMESPACE
