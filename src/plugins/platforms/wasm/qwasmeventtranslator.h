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

#ifndef QWASMEVENTTRANSLATOR_H
#define QWASMEVENTTRANSLATOR_H

#include <QtCore/qobject.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <emscripten/html5.h>
#include "qwasmwindow.h"
#include <QtGui/qtouchdevice.h>
#include <QHash>

QT_BEGIN_NAMESPACE

class QWindow;

class QWasmEventTranslator : public QObject
{
    Q_OBJECT

public:

    explicit QWasmEventTranslator(QWasmScreen *screen);
    virtual ~QWasmEventTranslator() override;

    static int keyboard_cb(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
    static int mouse_cb(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static int focus_cb(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);
    static int wheel_cb(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);

    static int touchCallback(int eventType, const EmscriptenTouchEvent *ev, void *userData);

    void processEvents();

private:
    friend class QWasmWindow;
    static void releaseIfCapturedWindow(QWasmWindow* window);
    static void releaseCapturedWindow();

    void initEventHandlers();
    QWasmScreen *screen();
    static Qt::Key translateEmscriptKey(const EmscriptenKeyboardEvent *emscriptKey);
    template <typename Event>
    static QFlags<Qt::KeyboardModifier> translatKeyModifier(const Event *event);
    static QFlags<Qt::KeyboardModifier> translateKeyboardEventModifier(const EmscriptenKeyboardEvent *keyEvent);
    static QFlags<Qt::KeyboardModifier> translateMouseEventModifier(const EmscriptenMouseEvent *mouseEvent);
    static Qt::MouseButton translateMouseButton(unsigned short button);

    static bool processKeyboard(int eventType, const EmscriptenKeyboardEvent *keyEvent);
    static Qt::Key translateDeadKey(Qt::Key deadKey, Qt::Key accentBaseKey);

    bool processMouseEnter(const EmscriptenMouseEvent *mouseEvent);
    bool processMouseLeave();
    bool processMouse(int eventType, const EmscriptenMouseEvent *mouseEvent);
    bool handleTouch(int eventType, const EmscriptenTouchEvent *touchEvent);

    bool ensureWindowCaptured(QWindow* window);
    static void maybeReleaseCapturedWindow();
    static void enterWindow(QWindow* window, const QPoint &localPoint, const QPoint &globalPoint);
    static void leaveWindow(QWindow* window);

    static quint64 getTimestamp();

private:
    static QMap <int, QPointF> pressedTouchIds;
    static QWasmEventTranslator *capturedTranslator;
    static QWindow *capturedWindow;
    static QWasmEventTranslator *mouseInTranslator;
    static QPointer<QWindow> mouseInWindow;
    static QPointer<QWindow> lastMouseInWindow;
    static QPointer<QWindow> draggedWindow;
    static Qt::MouseButtons pressedButtons;

    static QWasmWindow::ResizeMode resizeMode;
    static QPoint resizePoint;
    static QPoint lastMousePoint;
    static QRect resizeStartRect;
    static QTouchDevice *touchDevice;

    static Qt::Key m_emDeadKey;
    static bool m_emStickyDeadKey;
};

QT_END_NAMESPACE
#endif // QWASMEVENTTRANSLATOR_H
