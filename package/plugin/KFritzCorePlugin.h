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
#include "RecentCallsModel.h"
#include <QHash>
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
    Q_PROPERTY(QString currentCallerNumber READ currentCallerNumber NOTIFY callerInfoChanged)
    Q_PROPERTY(bool callerBlocked READ callerBlocked NOTIFY callerInfoChanged)
    Q_PROPERTY(bool callerUnknown READ callerUnknown NOTIFY callerInfoChanged)
    Q_PROPERTY(QStringList recentCalls READ recentCalls NOTIFY recentCallsChanged)
    Q_PROPERTY(QAbstractListModel *recentCallsModel READ recentCallsModel NOTIFY recentCallsChanged)

    QML_ELEMENT

public:
    explicit KFritzCorePlugin(QObject *parent = nullptr);
    QObject *callMonitor();
    bool callMonitorConnected() const;
    QString currentCaller() const;
    QStringList recentCalls() const;
    QString callerInfo() const;
    QString currentCallerNumber() const;
    bool callerBlocked() const;
    bool callerUnknown() const;
    QAbstractListModel *recentCallsModel() const;

    Q_INVOKABLE QVariantList getPhonebookList(const QString &host, int port, const QString &user, const QString &password);
    Q_INVOKABLE QVariantList listLocalPhonebooks();
    Q_INVOKABLE void connectToFritzBox();
    Q_INVOKABLE void setHost(const QString &host);
    Q_INVOKABLE QString resolveName(const QString &number) const;

    // ids assigned to each role (Settings), used to check an incoming number
    // against every phonebook in that role, not just one.
    Q_INVOKABLE void setContactsPhonebooks(const QVariantList &ids, int countryCode);
    Q_INVOKABLE void setBlocklistPhonebooks(const QVariantList &ids, int countryCode);

    // Adds `number` to phonebook `phonebookId` — the caller (QML) decides
    // which id that is (ContactsWriteTarget/BlocklistWriteTarget), so this
    // stays a thin, stateless wrapper.
    Q_INVOKABLE bool addPhonebookEntry(int phonebookId, const QString &name, const QString &number, const QString &type);

    // Fetches missed calls (Type 2) from the box's own call list since
    // `lastSeenId` (0 = everything the box still has), adds them to
    // recentCallsModel same as a live call would, and returns the new
    // highest id seen so QML can persist it (Plasmoid.configuration.
    // LastSeenCallId) for the next incremental fetch.
    Q_INVOKABLE int checkMissedCalls(int lastSeenId);

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
    bool isBlocked(const QString &number) const;

    FritzPhonebookFetcher m_fetcher;
    FritzCallMonitor *m_callMonitor = nullptr;
    QString m_host;
    QString m_callerInfo;
    QString m_currentCallerNumber;
    bool m_callerBlocked = false;
    bool m_callerUnknown = false;
    QHash<int, PhonebookLookup> m_lookups;
    QList<int> m_contactsIds;
    QList<int> m_blocklistIds;
    QStringList m_recentCalls;
    RecentCallsModel *m_recentCallsModel = nullptr;

private Q_SLOTS:

    void handleConnectionChanged();
    void handleCallerInfoChanged();
};
