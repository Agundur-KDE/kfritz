#include "FritzPhonebookFetcher.h"
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
    m_soap.setHost(host);
}
void FritzPhonebookFetcher::setPort(int port)
{
    m_port = port;
    m_soap.setPort(port);
}
void FritzPhonebookFetcher::setUsername(const QString &user)
{
    m_user = user;
    m_soap.setUsername(user);
}
void FritzPhonebookFetcher::setPassword(const QString &pass)
{
    m_pass = pass;
    m_soap.setPassword(pass);
}

QStringList FritzPhonebookFetcher::getPhonebookList()
{
    const QString body = u"<u:GetPhonebookList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>"_s;

    const QString response = m_soap.sendRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebookList"_s, body, u"/upnp/control/x_contact"_s);

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

    const QString response = m_soap.sendRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebook"_s, body, u"/upnp/control/x_contact"_s);

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

bool FritzPhonebookFetcher::addPhonebookEntry(int phonebookId, const QString &name, const QString &number, const QString &type)
{
    // Official contact schema, TR-064 Support – X_AVM-DE_OnTel, chapter 5.1
    // "Phonebook Content". No <uniqueid> tag => always adds a new entry
    // (SetPhonebookEntry with an empty NewPhonebookEntryID does the same).
    const QString entryData = u"<contact><category>0</category><person><realName>"_s + name.toHtmlEscaped() + u"</realName></person><telephony><services/><number type=\""_s
        + type.toHtmlEscaped() + u"\" prio=\"0\">"_s + number.toHtmlEscaped() + u"</number></telephony></contact>"_s;

    const QString body = u"<u:SetPhonebookEntry xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\">"
                         u"<NewPhonebookID>"_s
        + QString::number(phonebookId)
        + u"</NewPhonebookID>"
          u"<NewPhonebookEntryID></NewPhonebookEntryID>"
          u"<NewPhonebookEntryData>"_s
        + entryData.toHtmlEscaped() + u"</NewPhonebookEntryData></u:SetPhonebookEntry>"_s;

    const QString response = m_soap.sendRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"SetPhonebookEntry"_s, body, u"/upnp/control/x_contact"_s);

    if (response.contains(u"<errorCode>"_s)) {
        qWarning() << "SetPhonebookEntry failed:" << response;
        return false;
    }

    return true;
}

QList<FritzCallListEntry> FritzPhonebookFetcher::getCallList(int sinceId)
{
    QList<FritzCallListEntry> result;

    const QString body = u"<u:GetCallList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>"_s;
    const QString response = m_soap.sendRequest(u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetCallList"_s, body, u"/upnp/control/x_contact"_s);

    QString url;
    QXmlStreamReader urlXml(response);
    while (!urlXml.atEnd()) {
        urlXml.readNext();
        if (urlXml.isStartElement() && urlXml.name() == u"NewCallListURL"_s) {
            url = urlXml.readElementText().trimmed();
        }
    }

    if (url.isEmpty()) {
        qWarning() << "Empty SOAP response for GetCallList (feature disabled on the box?)";
        return result;
    }

    if (sinceId > 0) {
        url += (url.contains(u'?') ? u"&id="_s : u"?id="_s) + QString::number(sinceId);
    }

    QNetworkAccessManager nam;
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = nam.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "GetCallList download failed:" << reply->errorString();
        reply->deleteLater();
        return result;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    // Official schema, TR-064 Support - X_AVM-DE_OnTel, chapter 5.2 "Call
    // List Content": <root><Call><Id/><Type/><CallerNumber/><Name/><Date/>...
    QXmlStreamReader callXml(data);
    FritzCallListEntry current;
    bool inCall = false;
    while (!callXml.atEnd()) {
        callXml.readNext();
        if (callXml.isStartElement()) {
            const QStringView tag = callXml.name();
            if (tag == u"Call"_s) {
                inCall = true;
                current = FritzCallListEntry{};
            } else if (inCall && tag == u"Id"_s) {
                current.id = callXml.readElementText().toInt();
            } else if (inCall && tag == u"Type"_s) {
                current.type = callXml.readElementText().toInt();
            } else if (inCall && tag == u"CallerNumber"_s) {
                current.number = callXml.readElementText();
            } else if (inCall && tag == u"Name"_s) {
                current.name = callXml.readElementText();
            } else if (inCall && tag == u"Date"_s) {
                current.date = callXml.readElementText();
            }
        } else if (callXml.isEndElement() && callXml.name() == u"Call"_s) {
            inCall = false;
            result << current;
        }
    }

    if (callXml.hasError()) {
        qWarning() << "Call list XML parse error:" << callXml.errorString();
    }

    return result;
}
