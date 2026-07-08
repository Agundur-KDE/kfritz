/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "FritzSOAP.h"
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

    // Adds a new entry to phonebook `phonebookId` (SetPhonebookEntry, official
    // AVM contact XML schema — see TR-064 Support X_AVM-DE_OnTel, chapter 5.1).
    // Returns true if the SOAP call didn't report an error.
    bool addPhonebookEntry(int phonebookId, const QString &name, const QString &number, const QString &type = QStringLiteral("home"));

Q_SIGNALS:
    void phonebookDownloaded(int id, const QString &path);

private:
    QString getPhonebookUrl(int id);
    bool downloadPhonebook(int id, const QUrl &url);

    QString m_host, m_user, m_pass;
    int m_port = 49000;
    FritzSOAP m_soap;
};
