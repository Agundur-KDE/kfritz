#include "FritzPhonebookFetcher.h"

#include <QDebug>
#include <QEventLoop>
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

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"text/xml; charset=\"utf-8\""_s);

    const QString soapAction = u"\""_s + service + u"#"_s + action + u"\""_s;
    request.setRawHeader("SOAPACTION", soapAction.toUtf8());

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
