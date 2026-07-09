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

    // Called synchronously (see the nested QEventLoop below), some of it from
    // Component.onCompleted at plasmoid startup — without a timeout, an
    // unreachable box (e.g. network not up yet at login) would hang the
    // whole call, and with it the QML engine, for the OS's TCP connect
    // timeout (tens of seconds).
    request.setTransferTimeout(5000);

    // No preemptive Authorization header: FRITZ!Box's TR-064 endpoint expects
    // HTTP Digest, and setting the header manually would stop Qt from running
    // its own challenge-response negotiation (authenticationRequired below).
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
