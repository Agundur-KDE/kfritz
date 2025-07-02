/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FritzCallMonitor.h"
#include <QTcpSocket>

using namespace Qt::StringLiterals;

FritzCallMonitor::FritzCallMonitor(QObject *parent)
    : QObject(parent)
{
}

void FritzCallMonitor::setHost(const QString &host)
{
    m_host = host;
}
void FritzCallMonitor::setPort(int port)
{
    m_port = port;
}
void FritzCallMonitor::setUsername(const QString &user)
{
    m_user = user;
}
void FritzCallMonitor::setPassword(const QString &pass)
{
    m_pass = pass;
}
