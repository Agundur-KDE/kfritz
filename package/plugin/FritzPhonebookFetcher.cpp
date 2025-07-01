/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "FritzPhonebookFetcher.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QTextStream>
#include <QXmlStreamReader>

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

QString FritzPhonebookFetcher::sendSoapRequest(const QString& service, const QString& action, const QString& body, const QString& controlUrl)
{
    QNetworkAccessManager nam;
    QUrl url(QString("http://%1:%2%3").arg(m_host).arg(m_port).arg(controlUrl));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"utf-8\"");
    request.setRawHeader("SOAPACTION", QString("\"%1#%2\"").arg(service, action).toUtf8());

    QString envelope = QString(R"(
        <?xml version="1.0" encoding="utf-8"?>
        <s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
                    s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
            <s:Body>%1</s:Body>
        </s:Envelope>
    )")
                           .arg(body);

    QNetworkReply* reply = nam.post(request, envelope.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "SOAP Request failed:" << reply->errorString();
        reply->deleteLater();
        return QString();
    }

    QString response = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    return response;
}