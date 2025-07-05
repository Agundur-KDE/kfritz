/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "FritzCallMonitor.h"
#include "KFritzCorePlugin.h"
#include <QDebug>
#include <QNetworkInformation>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

using namespace Qt::StringLiterals;

FritzCallMonitor::FritzCallMonitor(QObject *parent)
    : QObject(parent)
{
    // m_callMonitor = new FritzCallMonitor(this);
    // m_callMonitor->setCorePlugin(this);
}

void FritzCallMonitor::setHost(const QString &host)
{
    m_host = host;
}

void FritzCallMonitor::connectToFritzBox()
{
    static int m_retryCount = 0;
    static constexpr int maxRetries = 12;

    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "✅ Bereits verbunden mit FritzBox.";
        return;
    }

    QNetworkInformation *netInfo = QNetworkInformation::instance();

    if (!netInfo) {
        qWarning() << "⚠️ Kein Netzwerk-Backend – versuche Verbindung trotzdem...";
    } else if (netInfo->reachability() < QNetworkInformation::Reachability::Online) {
        qDebug() << "📡 Netzwerk offline – neuer Versuch in 10 Sekunden...";
        QTimer::singleShot(10000, this, &FritzCallMonitor::connectToFritzBox);
        return;
    }

    if (m_retryCount >= maxRetries) {
        qWarning() << "❌ Verbindung zur FritzBox nach" << maxRetries << "Versuchen aufgegeben.";
        return;
    }

    if (m_host.isEmpty())
        m_host = u"fritz.box"_s;

    if (!m_socket) {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::readyRead, this, &FritzCallMonitor::onReadyRead);
        connect(m_socket, &QTcpSocket::errorOccurred, this, &FritzCallMonitor::onSocketError);
        connect(m_socket, &QTcpSocket::connected, this, &FritzCallMonitor::onConnected);
        connect(m_socket, &QTcpSocket::disconnected, this, &FritzCallMonitor::onDisconnected);
    }

    qDebug() << "🔌 Versuche Verbindung zur FritzBox @ " << m_host;
    m_retryCount++;
    m_socket->abort(); // falls vorher halb offen
    m_socket->connectToHost(m_host, 1012);
}

void FritzCallMonitor::onDisconnected()
{
    qWarning() << "🔌 Verbindung zur FritzBox verloren – versuche Reconnect...";
    m_connected = false;
    Q_EMIT connectedChanged(false); // 🔥 Jetzt auch der Disconnect signalisiert!
    QTimer::singleShot(5000, this, &FritzCallMonitor::connectToFritzBox);
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
