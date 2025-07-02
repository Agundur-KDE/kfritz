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
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QTextStream>
using namespace Qt::StringLiterals;

KFritzCorePlugin::KFritzCorePlugin(QObject *parent)
    : QObject(parent)
{
    connect(&m_fetcher, &FritzPhonebookFetcher::phonebookDownloaded, this, &KFritzCorePlugin::phonebookDownloaded);
}

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

#include "KFritzCorePlugin.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVariantList>
#include <QXmlStreamReader>

using namespace Qt::StringLiterals;

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
        QFile file(dir.absoluteFilePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << file.fileName();
            continue;
        }

        QXmlStreamReader xml(&file);
        QString phonebookName;
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == u"name") {
                phonebookName = xml.readElementText().trimmed();
                break;
            }
        }

        file.close();

        if (xml.hasError()) {
            qWarning() << "XML parse error in" << file.fileName() << ":" << xml.errorString();
            continue;
        }

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
        qDebug() << phonebookName;
        entry[QStringLiteral("name")] = phonebookName;
        result << entry;
    }

    return result;
}
