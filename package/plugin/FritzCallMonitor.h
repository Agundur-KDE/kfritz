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
    Q_PROPERTY(QString callerInfo READ callerInfo NOTIFY callerInfoChanged FINAL)
    QML_ELEMENT

public:
    explicit FritzCallMonitor(QObject *parent = nullptr);

    void setHost(const QString &host);
    void setPort(int port);
    void setUsername(const QString &user);
    void setPassword(const QString &pass);
    QString callerInfo() const;
    void connectToFritzBox();

Q_SIGNALS:
    void callerInfoChanged();

private Q_SLOTS:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket = nullptr;
    ;
    QString m_callerInfo;
    QString m_message;
    QString m_host, m_user, m_pass;
    int m_port = 49000;
};
