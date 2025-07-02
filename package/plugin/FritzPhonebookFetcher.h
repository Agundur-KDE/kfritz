/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QStringList>

class FritzPhonebookFetcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit FritzPhonebookFetcher(QObject *parent = nullptr);

    void setHost(const QString &host);
    void setPort(int port);
    void setUsername(const QString &user);
    void setPassword(const QString &pass);

    Q_INVOKABLE QStringList getPhonebookList();
    Q_INVOKABLE QString getPhonebookUrl(int phonebookId);
    Q_INVOKABLE bool downloadPhonebook(int id, const QUrl &url);

Q_SIGNALS:
    void phonebookDownloaded(int id, const QString &path);

private:
    QString m_host, m_user, m_pass;
    int m_port = 49000;

    QString sendSoapRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl);
};
