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

KFritzCorePlugin::KFritzCorePlugin(QObject* parent)
    : QObject(parent)
{
}

QStringList FritzPhonebookFetcher::getPhonebookList()
{
    QNetworkAccessManager nam;

    QString body = QString(
        "<u:GetPhonebookList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>");

    QString response = sendSoapRequest(
        "urn:dslforum-org:service:X_AVM-DE_OnTel:1",
        "GetPhonebookList",
        body,
        "/upnp/control/x_contact");

    // Beispielhafte einfache XML-Auswertung (todo: robuster machen)
    QStringList phonebooks;
    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "NewPhonebookList") {
            phonebooks = xml.readElementText().split(',');
            break;
        }
    }
    return phonebooks;
}