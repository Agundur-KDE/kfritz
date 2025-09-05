/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "FritzCallMonitor.h"
#include "KFritzCorePlugin.h"
#include <QDebug>
#include <QNetworkInformation>
#include <QNetworkProxy>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

using namespace Qt::StringLiterals;

FritzCallMonitor::FritzCallMonitor(QObject *parent)
    : QObject(parent)
{
    QTimer::singleShot(5'000, this, &FritzCallMonitor::connectToFritzBox);
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3 * 60 * 1000); // 3 Minuten
    m_reconnectTimer->setSingleShot(false);
    connect(m_reconnectTimer, &QTimer::timeout, this, &FritzCallMonitor::connectToFritzBox);

    if (!m_reconnectTimer) {
        m_reconnectTimer = new QTimer(this);
        m_reconnectTimer->setInterval(3 * 60 * 1000); // alle 3 Min
        m_reconnectTimer->setSingleShot(false);
        connect(m_reconnectTimer, &QTimer::timeout, this, &FritzCallMonitor::connectToFritzBox);
    }
}

void FritzCallMonitor::setHost(const QString &host)
{
    m_host = host.trimmed().isEmpty() ? u"fritz.box"_s : host.trimmed();
}

void FritzCallMonitor::connectToFritzBox()
{
    // Doppel-Connect vermeiden
    if (m_socket && (m_socket->state() == QAbstractSocket::ConnectingState || m_socket->state() == QAbstractSocket::ConnectedState)) {
        qDebug() << u"⚠ Bereits im Verbindungsaufbau/verbunden."_s;
        return;
    }

    // Socket vorbereiten
    if (!m_socket) {
        m_socket = new QTcpSocket(this);
        m_socket->setProxy(QNetworkProxy::NoProxy); // #include <QNetworkProxy>
        connect(m_socket, &QTcpSocket::readyRead, this, &FritzCallMonitor::onReadyRead);
        connect(m_socket, &QTcpSocket::errorOccurred, this, &FritzCallMonitor::onSocketError);
        connect(m_socket, &QTcpSocket::connected, this, &FritzCallMonitor::onConnected);
        connect(m_socket, &QTcpSocket::disconnected, this, &FritzCallMonitor::onDisconnected);
    } else {
        m_socket->setProxy(QNetworkProxy::NoProxy);
    }

    // QNetworkInformation darf kein Hard-Stop sein
    if (auto *netInfo = QNetworkInformation::instance()) {
        if (netInfo->reachability() < QNetworkInformation::Reachability::Online) {
            qDebug() << u"📡 Netzwerk offline – neuer Versuch in 10 Sekunden…"_s;
            QTimer::singleShot(10000, this, &FritzCallMonitor::connectToFritzBox);
            return;
        }
    } else {
        qWarning() << u"⚠ Kein QNetworkInformation-Backend – verbinde trotzdem…"_s;
        // kein return!
    }

    const QString host = m_host.trimmed().isEmpty() ? u"fritz.box"_s : m_host.trimmed();
    qDebug() << u"🔌 Versuche Verbindung zur FritzBox @"_s << host << u":1012"_s;

    m_socket->connectToHost(host, 1012);
}

void FritzCallMonitor::onDisconnected()
{
    qWarning() << "🔌 Verbindung zur FritzBox verloren – versuche Reconnect...";

    m_connected = false;
    Q_EMIT connectedChanged(false);

    if (!m_reconnectTimer->isActive()) {
        qDebug() << "⏳ Starte Reconnect-Timer (3 Min Intervall)";
        m_reconnectTimer->start(); // übernimmt alle Reconnects
    }
}

void FritzCallMonitor::onConnected()
{
    qDebug() << "🟢 Erfolgreich verbunden zur FritzBox.";
    m_retryCount = 0; // reset Retry-Zähler
    m_connected = true;
    Q_EMIT connectedChanged(true);
}

void FritzCallMonitor::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        QString lineStr = QString::fromUtf8(line);

        qDebug() << "Received:" << lineStr;

        if (lineStr.contains(QStringLiteral("RING"))) {
            QStringList parts = lineStr.split(QLatin1Char(';'));
            if (parts.size() >= 5) {
                QString number = parts.at(3);
                QString name = parts.at(4);

                m_callerInfo = QStringLiteral("%1").arg(number);

                if (m_corePlugin) {
                    m_corePlugin->handleIncomingCall(number);
                }

                Q_EMIT callerInfoChanged();
            }
        }
    }
}

void FritzCallMonitor::onSocketError(QAbstractSocket::SocketError socketError)
{
    if (!m_reconnectTimer->isActive()) {
        qDebug() << "⏳ Starte Reconnect-Timer (3 Min Intervall)";
        m_reconnectTimer->start();
    }
    qWarning() << "Socket error:" << m_socket->errorString();
    qWarning() << "Socket Error:" << socketError;
}

bool FritzCallMonitor::isConnected() const
{
    return m_connected;
}

QString FritzCallMonitor::callerInfo() const
{
    return m_callerInfo;
}

void FritzCallMonitor::setCorePlugin(KFritzCorePlugin *plugin)
{
    m_corePlugin = plugin;
}
