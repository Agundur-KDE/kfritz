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

    QStringList getPhonebookList();

Q_SIGNALS:
    void phonebookDownloaded(int id, const QString &path);

private:
    QString getPhonebookUrl(int id);
    bool downloadPhonebook(int id, const QUrl &url);

    QString m_host, m_user, m_pass;
    int m_port = 49000;

    QString sendSoapRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl);
};
