#include "FritzPhonebookFetcher.h"
#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
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

    QStringList ids = phonebooks;

    for (const QString &idStr : ids) {
        bool ok = false;
        int id = idStr.toInt(&ok);
        if (!ok) {
            qWarning() << "Invalid phonebook ID:" << idStr;
            continue;
        }

        QString url = getPhonebookUrl(id);
        if (!url.isEmpty()) {
            qDebug() << "Phonebook ID" << id << "→ URL:" << url;
            downloadPhonebook(id, QUrl(url));
        } else {
            qWarning() << "No URL for phonebook ID" << id;
        }
    }

    return phonebooks;
}

bool FritzPhonebookFetcher::downloadPhonebook(int id, const QUrl &url)
{
    qDebug() << "Downloading phonebook ID" << id << "from" << url;

    QNetworkAccessManager nam;
    QNetworkRequest request(url);
    QNetworkReply *reply = nam.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Download failed:" << reply->errorString();
        reply->deleteLater();
        return false;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + u"/phonebooks"_s;
    QDir().mkpath(baseDir);
    QString filePath = baseDir + u"/phonebook_"_s + QString::number(id) + u".xml"_s;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to write file:" << filePath;
        return false;
    }

    file.write(data);
    file.close();
    qDebug() << "Saved phonebook ID" << id << "to" << filePath;

    return true;
}

QString FritzPhonebookFetcher::getPhonebookUrl(int phonebookId)
{
    const QString body = u"<u:GetPhonebook xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\">"
                         u"<NewPhonebookID>"_s + QString::number(phonebookId) + u"</NewPhonebookID>"
                         u"</u:GetPhonebook>"_s;

    const QString response = sendSoapRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebook"_s, body, u"/upnp/control/x_contact"_s);

    if (response.isEmpty()) {
        qWarning() << "Empty SOAP response for GetPhonebook";
        return {};
    }

    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"NewPhonebookURL") {
            return xml.readElementText().trimmed();
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML Parse Error:" << xml.errorString();
    }

    return {};
}

QString FritzPhonebookFetcher::sendSoapRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl)
{
    const QUrl url = QUrl(u"http://"_s + m_host + u":"_s + QString::number(m_port) + controlUrl);

    QNetworkRequest request(url);

    // 🔐 Authorization
    QString credentials = m_user + u":"_s + m_pass;
    QByteArray auth = "Basic " + credentials.toUtf8().toBase64();

    request.setRawHeader("Authorization", auth);

    // 📄 Content & SOAP
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/xml; charset=\"utf-8\""));

    request.setRawHeader("SOAPACTION", "\"" + service.toUtf8() + "#" + action.toUtf8() + "\"");

    const QString envelope =
        uR"(<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
            s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
  <s:Body>)"_s
        + body + u"</s:Body></s:Envelope>"_s;

    QNetworkAccessManager nam;

    // 🔐 Optional: Auth-Fallback über QAuthenticator (nur wenn RawHeader nicht greift)
    QObject::connect(&nam, &QNetworkAccessManager::authenticationRequired, [this](QNetworkReply * /*reply*/, QAuthenticator *authenticator) {
        qDebug() << "authenticationRequired() triggered!";
        authenticator->setUser(m_user);
        authenticator->setPassword(m_pass);
    });

    QNetworkReply *reply = nam.post(request, envelope.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "SOAP request failed:" << reply->errorString();
    } else {
        result = QString::fromUtf8(reply->readAll());
    }

    reply->deleteLater();
    return result;
}
