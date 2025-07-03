/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QTcpSocket>

class FritzCallMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString callerInfo READ callerInfo NOTIFY callerInfoChanged FINAL)
    QML_ELEMENT

public:
    explicit FritzCallMonitor(QObject *parent = nullptr);

    void setHost(const QString &host);
    void connectToFritzBox();
    bool isConnected() const;
    QString callerInfo() const;

Q_SIGNALS:
    void connectedChanged();
    void callerInfoChanged();

private Q_SLOTS:
    void onReadyRead();
    void onConnected();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket = nullptr;
    bool m_connected = false;
    QString m_callerInfo;
    QString m_message;
    QString m_host, m_user, m_pass;
    int m_port = 49000;
};
