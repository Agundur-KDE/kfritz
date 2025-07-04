/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "FritzCallMonitor.h"
#include "FritzPhonebookFetcher.h"
#include "PhonebookCache.h"
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

class KFritzCorePlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *callMonitor READ callMonitor CONSTANT)
    Q_PROPERTY(bool callMonitorConnected READ callMonitorConnected NOTIFY callMonitorConnectedChanged)
    Q_PROPERTY(QString currentCaller READ currentCaller NOTIFY currentCallerChanged)
    Q_PROPERTY(QString callerInfo READ callerInfo NOTIFY callerInfoChanged)
    Q_PROPERTY(QStringList recentCalls READ recentCalls NOTIFY recentCallsChanged)
    QML_ELEMENT

public:
    explicit KFritzCorePlugin(QObject *parent = nullptr);
    QObject *callMonitor();
    bool callMonitorConnected() const;
    QString currentCaller() const;
    QStringList recentCalls() const;
    QString callerInfo() const;

    Q_INVOKABLE QVariantList getPhonebookList(const QString &host, int port, const QString &user, const QString &password);
    Q_INVOKABLE QVariantList listLocalPhonebooks();
    Q_INVOKABLE void connectToFritzBox();
    Q_INVOKABLE void setHost(const QString &host);
    Q_INVOKABLE QString resolveName(const QString &number) const;

public Q_SLOTS:
    void loadPhonebook(int phonebookId, int countryCode);
    void handleIncomingCall(const QString &number);

Q_SIGNALS:
    void phonebookDownloaded(int id, const QString &path);
    void callMonitorConnectedChanged();
    void currentCallerChanged();
    void callerInfoChanged();
    void recentCallsChanged();

private:
    FritzPhonebookFetcher m_fetcher;
    FritzCallMonitor *m_callMonitor = nullptr;
    QString m_host;
    QString m_callerInfo;
    PhonebookLookup m_lookup;
    QStringList m_recentCalls;

private Q_SLOTS:

    void handleConnectionChanged();
    void handleCallerInfoChanged();
};