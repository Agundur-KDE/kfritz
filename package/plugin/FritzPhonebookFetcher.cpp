#include "FritzPhonebookFetcher.h"

#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>

using namespace Qt::StringLiterals;

FritzPhonebookFetcher::FritzPhonebookFetcher(QObject *parent)
    : QObject(parent)
{
}

void FritzPhonebookFetcher::setHost(const QString &host)
{
    m_host = host;
}
void FritzPhonebookFetcher::setPort(int port)
{
    m_port = port;
}
void FritzPhonebookFetcher::setUsername(const QString &user)
{
    m_user = user;
}
void FritzPhonebookFetcher::setPassword(const QString &pass)
{
    m_pass = pass;
}

QStringList FritzPhonebookFetcher::getPhonebookList()
{
    const QString body = u"<u:GetPhonebookList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>"_s;

    const QString response = sendSoapRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebookList"_s, body, u"/upnp/control/x_contact"_s);

    QStringList phonebooks;
    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"NewPhonebookList"_s) {
            phonebooks = xml.readElementText().split(QStringLiteral(","), Qt::SkipEmptyParts);
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML Parse Error:" << xml.errorString();
    }

    return phonebooks;
}

QString FritzPhonebookFetcher::sendSoapRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl)
{
    QNetworkAccessManager nam;
    const QUrl url = QUrl(u"http://"_s + m_host + u":"_s + QString::number(m_port) + controlUrl);
    qDebug() << " URL:" << url;

    QNetworkRequest request(url);

    // 🔐 HTTP Basic Auth einfügen
    QString credentials = m_user + u":"_s + m_pass;

    qDebug() << " CRED:" << credentials;
    QByteArray auth = "Basic " + credentials.toUtf8().toBase64();
    request.setRawHeader("Authorization", auth);
    qDebug() << "Auth header:" << auth;

    // 📄 SOAP-spezifische Header
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"text/xml; charset=\"utf-8\""_s);
    QByteArray soapAction = "\"" + service.toUtf8() + "#" + action.toUtf8() + "\"";
    request.setRawHeader("SOAPACTION", soapAction);

    const QString envelope =
        uR"(<?xml version="1.0" encoding="utf-8"?>
        <s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
                    s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
            <s:Body>)"_s
        + body + u"</s:Body></s:Envelope>"_s;

    QNetworkReply *reply = nam.post(request, envelope.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString response;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "SOAP request failed:" << reply->errorString();
    } else {
        response = QString::fromUtf8(reply->readAll());
    }

    reply->deleteLater();
    return response;
}
