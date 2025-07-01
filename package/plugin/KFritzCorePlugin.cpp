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

KFritzCorePlugin::KFritzCorePlugin(QObject *parent)
    : QObject(parent)
{
}
QStringList KFritzCorePlugin::getPhonebookList(const QString &host, int port, const QString &user, const QString &password)
{
    m_fetcher.setHost(host);
    m_fetcher.setPort(port);
    m_fetcher.setUsername(user);
    m_fetcher.setPassword(password);
    return m_fetcher.getPhonebookList();
}