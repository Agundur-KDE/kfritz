/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FritzSOAP.h"

#include <QAuthenticator>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

using namespace Qt::StringLiterals;

void FritzSOAP::setHost(const QString &host)
{
    m_host = host;
}

void FritzSOAP::setPort(int port)
{
    m_port = port;
}

void FritzSOAP::setUsername(const QString &user)
{
    m_user = user;
}

void FritzSOAP::setPassword(const QString &pass)
{
    m_pass = pass;
}

QString FritzSOAP::sendRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl) const
{
    const QUrl url = QUrl(u"http://"_s + m_host + u":"_s + QString::number(m_port) + controlUrl);

    QNetworkRequest request(url);

    QString credentials = m_user + u":"_s + m_pass;
    QByteArray auth = "Basic " + credentials.toUtf8().toBase64();
    request.setRawHeader("Authorization", auth);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/xml; charset=\"utf-8\""));
    request.setRawHeader("SOAPACTION", "\"" + service.toUtf8() + "#" + action.toUtf8() + "\"");

    const QString envelope =
        uR"(<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
            s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
  <s:Body>)"_s
        + body + u"</s:Body></s:Envelope>"_s;

    QNetworkAccessManager nam;

    QObject::connect(&nam, &QNetworkAccessManager::authenticationRequired, [this](QNetworkReply * /*reply*/, QAuthenticator *authenticator) {
        authenticator->setUser(m_user);
        authenticator->setPassword(m_pass);
    });

    QNetworkReply *reply = nam.post(request, envelope.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "SOAP request failed:" << action << reply->errorString();
    } else {
        result = QString::fromUtf8(reply->readAll());
    }

    reply->deleteLater();
    return result;
}
