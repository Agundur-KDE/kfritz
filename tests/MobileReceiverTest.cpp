// SPDX-License-Identifier: GPL-3.0-or-later
#include "MobileCallReceiver.h"
#include "MobileProtocol.h"
#include <QTest>
#include <QSignalSpy>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QMessageAuthenticationCode>

class MobileReceiverTest:public QObject {
    Q_OBJECT
    QJsonObject vector;
    QByteArray v1(const QByteArray &nonce,const QByteArray &event,const QByteArray &call="test-call",const QByteArray &key=QByteArray()) {
        const auto secret=key.isEmpty()?QByteArray::fromHex(vector[QStringLiteral("key")].toString().toLatin1()):key;
        QByteArray payload="v1\ttest-device\t"+event+'\t'+call+"\t"+QByteArray("Test phone").toBase64();
        return payload+'\t'+QMessageAuthenticationCode::hash(QByteArray(nonce+'\n'+payload),secret,QCryptographicHash::Sha256).toHex();
    }
    QByteArray exchange(MobileCallReceiver &receiver,const QByteArray &event,const QByteArray &call="test-call",bool wrong=false) {
        QTcpSocket socket;socket.connectToHost(QHostAddress::LocalHost,quint16(receiver.port()));
        for(int i=0;i<100&&!socket.canReadLine();++i) QTest::qWait(5);
        if(!socket.canReadLine()) return {};
        QByteArray nonce=socket.readLine().trimmed();
        socket.write(v1(nonce,event,call,wrong?QByteArray(32,'x'):QByteArray())+'\n');socket.flush();
        for(int i=0;i<100&&!socket.canReadLine()&&socket.state()!=QAbstractSocket::UnconnectedState;++i)QTest::qWait(5);
        return socket.readAll();
    }
    void start(MobileCallReceiver &receiver) {
        QTcpServer probe;QVERIFY(probe.listen(QHostAddress::LocalHost,0));int port=probe.serverPort();probe.close();
        receiver.setPort(port);receiver.setPairingKey(vector[QStringLiteral("key")].toString());receiver.setEnabled(true);QVERIFY(receiver.listening());
    }
private Q_SLOTS:
    void initTestCase() { QFile file(QString::fromUtf8(WIRE_VECTOR_PATH));QVERIFY(file.open(QIODevice::ReadOnly));vector=QJsonDocument::fromJson(file.readAll()).object();QVERIFY(!vector.isEmpty()); }
    void decryptsIndependentAesGcmVector() {
        auto m=MobileProtocol::verify(vector[QStringLiteral("key")].toString(),vector[QStringLiteral("nonce")].toString().toLatin1(),vector[QStringLiteral("line")].toString().toUtf8());
        QVERIFY(m);QCOMPARE(m->name,vector[QStringLiteral("name")].toString());QCOMPARE(m->number,vector[QStringLiteral("number")].toString());QCOMPARE(m->label,vector[QStringLiteral("label")].toString());
    }
    void rejectsWrongKeyReplayAndTampering() {
        const auto key=vector[QStringLiteral("key")].toString(); const auto nonce=vector[QStringLiteral("nonce")].toString().toLatin1(); const auto line=vector[QStringLiteral("line")].toString().toUtf8();
        QVERIFY(!MobileProtocol::verify(QString(64,QLatin1Char('0')),nonce,line));
        QVERIFY(!MobileProtocol::verify(key,QByteArray(64,'b'),line));
        QVERIFY(!MobileProtocol::verify(key,nonce,QByteArray(line).replace("RING","END")));
    }
    void deduplicatesHeartbeatsAndKeepsConcurrentCallsSeparate() {
        MobileCallReceiver receiver;start(receiver);QSignalSpy rings(&receiver,&MobileCallReceiver::incomingCall),ends(&receiver,&MobileCallReceiver::callEnded);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("OK\n"));QCOMPARE(exchange(receiver,"RING"),QByteArray("OK\n"));QCOMPARE(rings.count(),1);
        QCOMPARE(exchange(receiver,"RING","second"),QByteArray("OK\n"));QCOMPARE(rings.count(),2);
        QCOMPARE(exchange(receiver,"END"),QByteArray("OK\n"));QCOMPARE(ends.count(),1);
        QCOMPARE(exchange(receiver,"END","second"),QByteArray("OK\n"));QCOMPARE(ends.count(),2);
    }
    void rejectsUnauthenticatedNetworkMessagesAndInvalidKey() {
        MobileCallReceiver receiver;receiver.setEnabled(true);QVERIFY(!receiver.listening());start(receiver);
        QSignalSpy rings(&receiver,&MobileCallReceiver::incomingCall);
        QVERIFY(exchange(receiver,"RING","test-call",true).isEmpty());QCOMPARE(rings.count(),0);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("OK\n"));QCOMPARE(rings.count(),1);
    }
    void disablingEndsTrackedCalls() {
        MobileCallReceiver receiver;start(receiver);QSignalSpy ends(&receiver,&MobileCallReceiver::callEnded);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("OK\n"));receiver.setEnabled(false);QVERIFY(!receiver.listening());QCOMPARE(ends.count(),1);
    }
};
QTEST_GUILESS_MAIN(MobileReceiverTest)
#include "MobileReceiverTest.moc"
