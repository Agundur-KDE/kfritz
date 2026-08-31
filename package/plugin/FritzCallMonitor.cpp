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
#include <algorithm>

using namespace Qt::StringLiterals;

FritzCallMonitor::FritzCallMonitor(QObject *parent)
    : QObject(parent)
{
    QTimer::singleShot(5'000, this, &FritzCallMonitor::connectToFritzBox);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &FritzCallMonitor::connectToFritzBox);
}

int FritzCallMonitor::nextRetryDelayMs()
{
    // Exponentieller Backoff (5s, 10s, 20s, ... ) gedeckelt bei 3 Minuten,
    // damit ein kurzer Netzwerk-Hänger schnell nachversucht, ein länger
    // nicht erreichbarer Router aber nicht im Sekundentakt anklopft.
    static constexpr int baseDelayMs = 5'000;
    static constexpr int maxDelayMs = 3 * 60 * 1000;

    const int uncapped = baseDelayMs << std::min(m_retryCount, 10);
    return std::min(uncapped, maxDelayMs);
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

    const int delayMs = nextRetryDelayMs();
    ++m_retryCount;
    qDebug() << "⏳ Reconnect-Versuch in" << delayMs / 1000 << "Sekunden";
    m_reconnectTimer->start(delayMs);
}

void FritzCallMonitor::onConnected()
{
    qDebug() << "🟢 Erfolgreich verbunden zur FritzBox.";
    m_reconnectTimer->stop(); // kein hängender Backoff-Versuch nach Erfolg
    m_retryCount = 0;
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

    // Ein gescheiterter Erstverbindungsversuch löst kein disconnected() aus
    // (der Socket war nie verbunden) — hier also selbst nachplanen, aber nur
    // wenn onDisconnected() nicht schon einen Backoff-Versuch laufen hat.
    if (!m_reconnectTimer->isActive()) {
        const int delayMs = nextRetryDelayMs();
        ++m_retryCount;
        qDebug() << "⏳ Reconnect-Versuch in" << delayMs / 1000 << "Sekunden";
        m_reconnectTimer->start(delayMs);
    }
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
