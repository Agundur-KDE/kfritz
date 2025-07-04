/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FritzCallMonitor.h"
#include <QDebug>
#include <QObject>
#include <QTcpSocket>

using namespace Qt::StringLiterals;

FritzCallMonitor::FritzCallMonitor(QObject *parent)
    : QObject(parent)
{
}

void FritzCallMonitor::setHost(const QString &host)
{
    m_host = host;
}

void FritzCallMonitor::connectToFritzBox()
{
    qDebug() << "Connecting to FritzBox CallMonitor..." << m_host;
    if (!m_socket) {
        m_socket = new QTcpSocket(this);
    }

    connect(m_socket, &QTcpSocket::readyRead, this, &FritzCallMonitor::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &FritzCallMonitor::onSocketError);
    connect(m_socket, &QTcpSocket::connected, this, &FritzCallMonitor::onConnected);

    m_socket->connectToHost(m_host, 1012);
}

// QString FritzCallMonitor::callerInfo() const
// {
//     return m_callerInfo;
// }

void FritzCallMonitor::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        QString lineStr = QString::fromUtf8(line);

        qDebug() << "Received:" << lineStr;

        if (lineStr.contains(QStringLiteral("RING"))) {
            // Beispielzeile: 14.06.24 10:55:01;RING;0;01701234567;Max Mustermann;
            QStringList parts = lineStr.split(QLatin1Char(';'));
            if (parts.size() >= 5) {
                QString number = parts.at(3);
                QString name = parts.at(4);

                m_callerInfo = QStringLiteral("%1").arg(number);
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

void FritzCallMonitor::onConnected()
{
    m_connected = true;
    Q_EMIT connectedChanged();
    qDebug() << "✔️ FritzCallMonitor: Verbunden zur FritzBox.";
}

bool FritzCallMonitor::isConnected() const
{
    return m_connected;
}

QString FritzCallMonitor::callerInfo() const
{
    return m_callerInfo;
}