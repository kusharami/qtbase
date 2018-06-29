/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
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

#ifndef QNETWORKREPLYWASMIMPL_H
#define QNETWORKREPLYWASMIMPL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Network Access API.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtNetwork/private/qtnetworkglobal_p.h>
#include "qnetworkreply.h"
#include "qnetworkreply_p.h"
#include "qnetworkaccessmanager.h"
#include <QFile>
#include <private/qabstractfileengine_p.h>

#include <emscripten.h>
#include <emscripten/html5.h>

QT_BEGIN_NAMESPACE

class QIODevice;
//class QNetworkAccessBackend;

class QNetworkReplyWasmImplPrivate;
class QNetworkReplyWasmImpl: public QNetworkReply
{
    Q_OBJECT
public:
    QNetworkReplyWasmImpl(QObject *parent = 0/*QNetworkAccessManager *manager, const QNetworkRequest &req, const QNetworkAccessManager::Operation op, QIODevice *outgoingData*/);
    ~QNetworkReplyWasmImpl();
    virtual void abort() Q_DECL_OVERRIDE;

    // reimplemented from QNetworkReply
    virtual void close() Q_DECL_OVERRIDE;
    virtual qint64 bytesAvailable() const Q_DECL_OVERRIDE;
    virtual bool isSequential () const Q_DECL_OVERRIDE;
    qint64 size() const Q_DECL_OVERRIDE;

    virtual qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE;

    void setup(QNetworkAccessManager::Operation op, const QNetworkRequest &request,
               QIODevice *outgoingData);

    Q_DECLARE_PRIVATE(QNetworkReplyWasmImpl)

    Q_PRIVATE_SLOT(d_func(), void emitReplyError(QNetworkReply::NetworkError errorCode, const QString &errorString))
    Q_PRIVATE_SLOT(d_func(), void emitDataReadProgress(qint64 done, qint64 total))
    Q_PRIVATE_SLOT(d_func(), void dataReceived(char *buffer, int bufferSize))


private:
    QByteArray methodName() const;

};

class QNetworkReplyWasmImplPrivate: public QNetworkReplyPrivate
{
public:
    QNetworkReplyWasmImplPrivate();
    ~QNetworkReplyWasmImplPrivate();

    QNetworkAccessManagerPrivate *managerPrivate;
    void doSendRequest();

    void jsRequest(const QString &verb, const QString &url, void *, void *, void *, void *);

    static void onLoadCallback(void *data, int statusCode, int statusReason, int readyState, int textBuffer, int size);
    static void onProgressCallback(void *data, int done, int bytesTotal, uint timestamp);
    static void onRequestErrorCallback(void *data, int statusCode, int statusReason);
    static void onStateChangedCallback(int status);
    static void onResponseHeadersCallback(void *data, int headers);

    void emitReplyError(QNetworkReply::NetworkError errorCode, const QString &);
    void emitDataReadProgress(qint64 done, qint64 total);
    void dataReceived(char *buffer, int bufferSize);
    void headersReceived(char *buffer);

    void setup(QNetworkAccessManager::Operation op, const QNetworkRequest &request,
               QIODevice *outgoingData);

    State state;
    void _q_bufferOutgoingData();
    void _q_bufferOutgoingDataFinished();

    QSharedPointer<QAtomicInt> pendingDownloadData;
    QSharedPointer<QAtomicInt> pendingDownloadProgress;
   // QNetworkAccessBackend *backend;

    qint64 bytesDownloaded;
    qint64 bytesBuffered;

    qint64 downloadBufferReadPosition;
    qint64 downloadBufferCurrentSize;
    qint64 totalDownloadSize;
    qint64 percentFinished;
    QByteArray downloadBuffer;

    QIODevice *outgoingData;
    QSharedPointer<QRingBuffer> outgoingDataBuffer;

    static QNetworkReply::NetworkError  statusCodeFromHttp(int httpStatusCode, const QUrl &url);
    Q_DECLARE_PUBLIC(QNetworkReplyWasmImpl)
};

QT_END_NAMESPACE

//Q_DECLARE_METATYPE(QNetworkRequest::KnownHeaders)

#endif // QNETWORKREPLYWASMIMPL_H