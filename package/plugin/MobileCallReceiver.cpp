// SPDX-License-Identifier: GPL-3.0-or-later
#include "MobileCallReceiver.h"
#include <KLocalizedString>
#include <QTcpSocket>

MobileCallReceiver::MobileCallReceiver(QObject *parent):QObject(parent) {
    m_clock.start();
    connect(&m_server,&QTcpServer::newConnection,this,&MobileCallReceiver::accept);
    m_expiry.setInterval(10000);
    connect(&m_expiry,&QTimer::timeout,this,[this] {
        const auto keys=m_calls.keys();
        for (const auto &id:keys) if (m_clock.elapsed()-m_calls.value(id).seen>90000) { m_calls.remove(id); Q_EMIT callEnded(id); }
    });
    m_expiry.start();
}
QString MobileCallReceiver::status() const {
    if (m_status==QLatin1String("listening")) return i18nd("plasma_applet_de.agundur.kfritz","Receiving mobile calls on port %1",m_port);
    if (m_status==QLatin1String("key")) return i18nd("plasma_applet_de.agundur.kfritz","Enter a valid pairing key");
    if (m_status==QLatin1String("failed")) return i18nd("plasma_applet_de.agundur.kfritz","Could not open the mobile call port");
    return i18nd("plasma_applet_de.agundur.kfritz","Mobile call reception is disabled");
}
void MobileCallReceiver::setEnabled(bool value) { if (m_enabled!=value) { m_enabled=value; restart(); } }
void MobileCallReceiver::setPairingKey(const QString &value) { if (m_key!=value) { m_key=value; restart(); } }
void MobileCallReceiver::setPort(int value) { if (value>=1 && value<=65535 && m_port!=value) { m_port=value; restart(); } }
QString MobileCallReceiver::newPairingKey() const { return QString::fromLatin1(MobileProtocol::newChallenge()); }
void MobileCallReceiver::restart() {
    m_server.close();
    // Abort in-flight challenges after key rotation or disabling the listener.
    for (QTcpSocket *socket:findChildren<QTcpSocket*>()) socket->abort();
    const auto ids=m_calls.keys();m_calls.clear();
    for (const auto &id:ids) Q_EMIT callEnded(id);
    m_status=QStringLiteral("disabled");
    if (m_enabled) {
        if (!MobileProtocol::validKey(m_key)) m_status=QStringLiteral("key");
        else m_status=m_server.listen(QHostAddress::AnyIPv4,quint16(m_port)) ? QStringLiteral("listening") : QStringLiteral("failed");
    }
    Q_EMIT changed();
}
void MobileCallReceiver::accept() {
    while (m_server.hasPendingConnections()) {
        QTcpSocket *socket=m_server.nextPendingConnection();socket->setParent(this);
        if (m_clients>=8) { socket->abort(); socket->deleteLater(); continue; }
        ++m_clients;
        connect(socket,&QTcpSocket::disconnected,this,[this,socket] { --m_clients; socket->deleteLater(); });
        const QByteArray challenge=MobileProtocol::newChallenge();
        const QString key=m_key;
        socket->write(challenge+'\n');
        QTimer::singleShot(3000,socket,[socket] { socket->abort(); });
        connect(socket,&QTcpSocket::readyRead,this,[this,socket,challenge,key] {
            if (socket->property("processed").toBool()) return;
            if (socket->bytesAvailable()>4096) { socket->abort(); return; }
            if (!socket->canReadLine()) return;
            socket->setProperty("processed",true);
            QByteArray line=socket->readLine(4097);line.chop(1);
                if (!m_enabled || m_key!=key) { socket->abort(); return; }
                const auto message=MobileProtocol::verify(key,challenge,line);
                if (!message) { socket->abort(); return; }
                if(!process(*message)) { socket->abort();return; }
                socket->write(MobileProtocol::acknowledgement(key,challenge,line)+'\n');socket->disconnectFromHost();
        });
    }
}
bool MobileCallReceiver::process(const MobileMessage &m) {
    if (m.event==QLatin1String("TEST")) { Q_EMIT testMessage(m.label); return true; }
    const QString id=m.device+QLatin1Char('/')+m.call;
    if (m.event==QLatin1String("END")) { if (m_calls.remove(id)) Q_EMIT callEnded(id); return true; }
    if (!m_calls.contains(id)) {
        if (m_calls.size()>=32) return false;
        if (m.event!=QLatin1String("RING")) return false;
        m_calls.insert(id,Call{m,m_clock.elapsed(),false});
        Q_EMIT incomingCall(id,m.name,m.number,m.label);return true;
    }
    auto &call=m_calls[id];call.seen=m_clock.elapsed();
    const bool updated=call.message.name!=m.name || call.message.number!=m.number || call.message.label!=m.label;
    call.message=m;
    if (m.event==QLatin1String("CONNECT")) call.answered=true;
    if (updated) Q_EMIT callerUpdated(id,m.name,m.number,m.label);
    return true;
}
