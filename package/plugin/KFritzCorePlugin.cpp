/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "KFritzCorePlugin.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QTextStream>
#include <QVariantList>
// #include <QXmlStreamReader>
#include <KLocalizedString>
#include <KNotification>
#include <QDomDocument>

using namespace Qt::StringLiterals;

KFritzCorePlugin::KFritzCorePlugin(QObject *parent)
    : QObject(parent)

{
    m_callMonitor = new FritzCallMonitor(this);
    m_callMonitor->setCorePlugin(this);

    connect(&m_fetcher, &FritzPhonebookFetcher::phonebookDownloaded, this, &KFritzCorePlugin::phonebookDownloaded);
    connect(m_callMonitor, &FritzCallMonitor::connectedChanged, this, &KFritzCorePlugin::handleConnectionChanged);
    connect(m_callMonitor, &FritzCallMonitor::callerInfoChanged, this, &KFritzCorePlugin::handleCallerInfoChanged);
}

/************************* Phonebook *******************************/

QVariantList KFritzCorePlugin::getPhonebookList(const QString &host, int port, const QString &user, const QString &password)
{
    m_fetcher.setHost(host);
    m_fetcher.setPort(port);
    m_fetcher.setUsername(user);
    m_fetcher.setPassword(password);

    QStringList list = m_fetcher.getPhonebookList();
    QVariantList result;
    for (const QString &s : list)
        result << s;
    return result;
}

QString extractPhonebookNameFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Could not parse XML content from" << filePath;
        return {};
    }

    QDomElement root = doc.documentElement();
    QDomNodeList phonebooks = root.elementsByTagName(u"phonebook"_s);
    if (phonebooks.isEmpty())
        return {};

    QDomElement pb = phonebooks.at(0).toElement();
    return pb.attribute(u"name"_s).trimmed();
}

QVariantList KFritzCorePlugin::listLocalPhonebooks()
{
    QVariantList result;

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + u"/phonebooks"_s;
    QDir dir(baseDir);
    if (!dir.exists()) {
        qWarning() << "Phonebook directory does not exist:" << baseDir;
        return result;
    }

    QStringList files = dir.entryList(QStringList() << u"phonebook_*.xml"_s, QDir::Files);
    for (const QString &fileName : files) {
        QString fullPath = dir.absoluteFilePath(fileName);
        QString phonebookName = extractPhonebookNameFromFile(fullPath);

        QRegularExpression re(u"phonebook_(\\d+)\\.xml"_s);
        QRegularExpressionMatch match = re.match(fileName);
        if (!match.hasMatch()) {
            qWarning() << "Filename doesn't match expected format:" << fileName;
            continue;
        }

        bool ok = false;
        int id = match.captured(1).toInt(&ok);
        if (!ok) {
            qWarning() << "Invalid ID extracted from filename:" << fileName;
            continue;
        }

        QVariantMap entry;
        entry[QStringLiteral("id")] = id;
        qDebug() << id;
        entry[QStringLiteral("name")] = phonebookName;
        result << entry;
    }

    return result;
}

/************************* Callmonitor *******************************/

void KFritzCorePlugin::setHost(const QString &host)
{
    m_host = host;
}

QObject *KFritzCorePlugin::callMonitor()
{
    return m_callMonitor;
}

void KFritzCorePlugin::connectToFritzBox()
{
    if (!m_host.isEmpty()) {
        m_callMonitor->setHost(m_host);
    } else {
        m_callMonitor->setHost(QStringLiteral("fritz.box"));
    }

    m_callMonitor->connectToFritzBox();
}

bool KFritzCorePlugin::callMonitorConnected() const
{
    return m_callMonitor->isConnected();
}

QString KFritzCorePlugin::currentCaller() const
{
    return m_callMonitor->callerInfo();
}

void KFritzCorePlugin::handleConnectionChanged()
{
    Q_EMIT callMonitorConnectedChanged();
}

void KFritzCorePlugin::handleCallerInfoChanged()
{
    Q_EMIT currentCallerChanged();
}

/************************* Number matcher *******************************/

QString KFritzCorePlugin::callerInfo() const
{
    return m_callerInfo;
}

void KFritzCorePlugin::loadPhonebook(int phonebookId, int countryCode)
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + u"/phonebooks"_s;
    QString path = baseDir + u"/phonebook_"_s + QString::number(phonebookId) + u".xml"_s;

    qDebug() << "Lade Telefonbuch:" << path;
    QString countryPrefix = u"+" + QString::number(countryCode);
    m_lookup.loadFromFile(path, countryPrefix);
}

QString KFritzCorePlugin::resolveName(const QString &number) const
{
    return m_lookup.resolveName(number);
}
/************************* Call history *******************************/

QStringList KFritzCorePlugin::recentCalls() const
{
    return m_recentCalls;
}

void KFritzCorePlugin::handleIncomingCall(const QString &number)
{
    QString name = m_lookup.resolveName(number);
    QString timestamp = QDateTime::currentDateTime().toString(u"hh:mm:ss"_s);

    QString message = name.isEmpty() ? i18n("Incoming call from %1", number) : i18n("Incoming call from %1 (%2)", name, number);

    auto *notification = new KNotification(QStringLiteral("incomingCall"), KNotification::CloseOnTimeout);
    notification->setComponentName(QStringLiteral("kfritz"));
    notification->setTitle(i18n("FritzBox Call"));
    notification->setText(message);
    notification->setIconName(QStringLiteral("call-incoming")); // z. B. phone-incoming-symbolic
    notification->sendEvent();

    QString entry = timestamp + u" – "_s + (!name.isEmpty() ? name + u" (" + number + u")" : number);

    m_callerInfo = !name.isEmpty() ? name + u" (" + number + u")" : number;
    Q_EMIT callerInfoChanged();

    m_recentCalls.prepend(entry);
    if (m_recentCalls.size() > 20) // nur letzte 20 anzeigen
        m_recentCalls.removeLast();
    qDebug() << "📞 :" << number << "→ recentCalls:" << m_recentCalls;

    Q_EMIT recentCallsChanged();
}
