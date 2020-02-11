/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwasmdrag.h"

#include "qbitmap.h"
#include "qdrag.h"
#include "qpixmap.h"
#include "qevent.h"
#include "qguiapplication.h"
#include "qpoint.h"
#include "qbuffer.h"
#include "qimage.h"

#include <qpa/qplatformscreen.h>
#include <qpa/qplatformwindow.h>

#include <private/qguiapplication_p.h>
#include <private/qdnd_p.h>

#include <private/qshapedpixmapdndwindow_p.h>
#include <private/qhighdpiscaling_p.h>

QT_BEGIN_NAMESPACE

static QWindow* topLevelAt(const QPoint &pos)
{
    QWindowList list = QGuiApplication::topLevelWindows();
    for (int i = list.count()-1; i >= 0; --i) {
        QWindow *w = list.at(i);
        if (w->isVisible() && w->handle() && w->geometry().contains(pos) && !qobject_cast<QShapedPixmapWindow*>(w))
            return w;
    }
    return 0;
}

static inline QPoint getNativeMousePos(QMouseEvent *e, QWindow *window)
{
    return QHighDpi::toNativePixels(e->globalPos(), window);
}

static void sendDragLeave(QWindow *window)
{
    QWindowSystemInterface::handleDrag(window, nullptr, QPoint(), Qt::IgnoreAction, 0, 0);
}

static inline QPoint fromNativeGlobalPixels(const QPoint &point)
{
#ifndef QT_NO_HIGHDPISCALING
    QPoint res = point;
    if (QHighDpiScaling::isActive()) {
        for (const QScreen *s : qAsConst(QGuiApplicationPrivate::screen_list)) {
            if (s->handle()->geometry().contains(point)) {
                res = QHighDpi::fromNativePixels(point, s);
                break;
            }
        }
    }
    return res;
#else
    return point;
#endif
}

QWasmDrag::~QWasmDrag()
{
    delete m_drag;
}

bool QWasmDrag::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    switch (e->type()) {
        case QEvent::ShortcutOverride:
            // prevent accelerators from firing while dragging
            e->accept();
            return true;

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress) {
                cancelDrag();
            }
            return true; // Eat all key events
        }

        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
        {
            return true;
        }

        case QEvent::MouseMove:
        {
            auto mouseMove = static_cast<QMouseEvent *>(e);
            QPoint nativePosition = getNativeMousePos(mouseMove, m_drag_icon_window);
            move(nativePosition, mouseMove->buttons(), mouseMove->modifiers());
            return true; // Eat all mouse move events
        }
        case QEvent::MouseButtonRelease:
        {
            auto mouseRelease = static_cast<QMouseEvent *>(e);
            if (mouseRelease->button() != Qt::LeftButton)
                return true;

            disableEventFilter();
            if (canDrop()) {
                QPoint nativePosition = getNativeMousePos(mouseRelease, m_drag_icon_window);
                drop(nativePosition, mouseRelease->buttons(), mouseRelease->modifiers());
            } else {
                cancelDrag();
            }
            break;
        }

        default:
             break;
    }
    return false;
}

bool QWasmDrag::ownsDragObject() const
{
    return true;
}

Qt::DropAction QWasmDrag::drag(QDrag *o)
{
    m_drag = o;
    m_executed_drop_action = Qt::IgnoreAction;
    m_can_drop = false;

    startDrag();
    return m_executed_drop_action;
}

void QWasmDrag::cancelDrag()
{
    cancel();
}

void QWasmDrag::startDrag()
{
    setExecutedDropAction(Qt::IgnoreAction);

    m_sourceWindow = topLevelAt(QCursor::pos());
    m_screen = m_sourceWindow->screen();
    QBasicDrag::startDrag();
    m_windowUnderCursor = m_sourceWindow;
    if (m_sourceWindow) {
        auto nativePixelPos = QHighDpi::toNativePixels(QCursor::pos(), m_sourceWindow);
        move(nativePixelPos, QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
    } else {
        setCanDrop(false);
        updateCursor(Qt::IgnoreAction);
    }
}

void QWasmDrag::cancel()
{
    QBasicDrag::cancel();
    if (m_drag && m_sourceWindow) {
        sendDragLeave(m_sourceWindow);
        m_sourceWindow = nullptr;
    }
    endDrag();
}

void QWasmDrag::move(const QPoint &nativeGlobalPos, Qt::MouseButtons buttons,
                       Qt::KeyboardModifiers modifiers)
{
    QPoint globalPos = fromNativeGlobalPixels(nativeGlobalPos);
    moveShapedPixmapWindow(globalPos);
    QWindow *window = topLevelAt(globalPos);

    if (!window || window != m_windowUnderCursor) {
        if (m_windowUnderCursor)
            sendDragLeave(m_windowUnderCursor);
        m_windowUnderCursor = window;
        if (!window) {
            setCanDrop(false);
            updateCursor(Qt::IgnoreAction);
            return;
        }

        //m_drag_icon_window->setScreen(window->screen());
    }

    const QPoint pos = nativeGlobalPos - window->handle()->geometry().topLeft();
    const QPlatformDragQtResponse qt_response = QWindowSystemInterface::handleDrag(
                window, m_drag->mimeData(), pos, m_drag->supportedActions(),
                buttons, modifiers);

    setCanDrop(qt_response.isAccepted());
    updateCursor(qt_response.acceptedAction());
}

void QWasmDrag::drop(const QPoint &nativeGlobalPos, Qt::MouseButtons buttons,
                       Qt::KeyboardModifiers modifiers)
{
    QPoint globalPos = fromNativeGlobalPixels(nativeGlobalPos);

    QBasicDrag::drop(nativeGlobalPos, buttons, modifiers);
    QWindow *window = topLevelAt(globalPos);
    if (window) {
        const QPoint pos = nativeGlobalPos - window->handle()->geometry().topLeft();
        const QPlatformDropQtResponse response = QWindowSystemInterface::handleDrop(
                    window, m_drag->mimeData(), pos, m_drag->supportedActions(),
                    buttons, modifiers);
        if (response.isAccepted()) {
            setExecutedDropAction(response.acceptedAction());
        } else {
            setExecutedDropAction(Qt::IgnoreAction);
        }
    }

    endDrag();
}

void QWasmDrag::endDrag()
{
    if (m_drag) {
        auto drag = m_drag;
        QBasicDrag::endDrag();
        drag->deleteLater();
    }
}

QT_END_NAMESPACE
