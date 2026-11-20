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
        const auto frame=v1(nonce,event,call,wrong?QByteArray(32,'x'):QByteArray());
        socket.write(frame+'\n');socket.flush();
        for(int i=0;i<100&&!socket.canReadLine()&&socket.state()!=QAbstractSocket::UnconnectedState;++i)QTest::qWait(5);
        const auto reply=socket.readAll();
        const auto expected=MobileProtocol::acknowledgement(vector[QStringLiteral("key")].toString(),nonce,frame)+'\n';
        return reply==expected?QByteArray("verified\n"):reply;
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
    void acknowledgementMatchesIndependentAndroidVector() {
        const auto key=vector[QStringLiteral("key")].toString(),nonce=vector[QStringLiteral("nonce")].toString();
        const auto line=vector[QStringLiteral("line")].toString().toUtf8();
        QCOMPARE(MobileProtocol::acknowledgement(key,nonce.toLatin1(),line),QByteArray("ACK\t76782a1618c759106a03380beb74f0eb50a693ee5c788d5e6491c76735842d58"));
        QVERIFY(MobileProtocol::acknowledgement(key,nonce.toLatin1(),line)!=MobileProtocol::acknowledgement(key,QByteArray(64,'b'),line));
    }
    void rejectsWrongKeyReplayAndTampering() {
        const auto key=vector[QStringLiteral("key")].toString(); const auto nonce=vector[QStringLiteral("nonce")].toString().toLatin1(); const auto line=vector[QStringLiteral("line")].toString().toUtf8();
        QVERIFY(!MobileProtocol::verify(QString(64,QLatin1Char('0')),nonce,line));
        QVERIFY(!MobileProtocol::verify(key,QByteArray(64,'b'),line));
        QVERIFY(!MobileProtocol::verify(key,nonce,QByteArray(line).replace("RING","END")));
    }
    void deduplicatesHeartbeatsAndKeepsConcurrentCallsSeparate() {
        MobileCallReceiver receiver;start(receiver);QSignalSpy rings(&receiver,&MobileCallReceiver::incomingCall),ends(&receiver,&MobileCallReceiver::callEnded);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("verified\n"));QCOMPARE(exchange(receiver,"RING"),QByteArray("verified\n"));QCOMPARE(rings.count(),1);
        QCOMPARE(exchange(receiver,"RING","second"),QByteArray("verified\n"));QCOMPARE(rings.count(),2);
        QCOMPARE(exchange(receiver,"END"),QByteArray("verified\n"));QCOMPARE(ends.count(),1);
        QCOMPARE(exchange(receiver,"END","second"),QByteArray("verified\n"));QCOMPARE(ends.count(),2);
    }
    void rejectsUnauthenticatedNetworkMessagesAndInvalidKey() {
        MobileCallReceiver receiver;receiver.setEnabled(true);QVERIFY(!receiver.listening());start(receiver);
        QSignalSpy rings(&receiver,&MobileCallReceiver::incomingCall);
        QVERIFY(exchange(receiver,"RING","test-call",true).isEmpty());QCOMPARE(rings.count(),0);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("verified\n"));QCOMPARE(rings.count(),1);
    }
    void disablingEndsTrackedCalls() {
        MobileCallReceiver receiver;start(receiver);QSignalSpy ends(&receiver,&MobileCallReceiver::callEnded);
        QCOMPARE(exchange(receiver,"RING"),QByteArray("verified\n"));receiver.setEnabled(false);QVERIFY(!receiver.listening());QCOMPARE(ends.count(),1);
    }
    void rejectedCapacityAndUnknownConnectAreNotAcknowledged() {
        MobileCallReceiver receiver;start(receiver);
        QVERIFY(exchange(receiver,"CONNECT","unknown").isEmpty());
        QCOMPARE(exchange(receiver,"END","unknown"),QByteArray("verified\n"));
        for(int i=0;i<32;i++) QCOMPARE(exchange(receiver,"RING",QByteArray::number(i)),QByteArray("verified\n"));
        QVERIFY(exchange(receiver,"RING","overflow").isEmpty());
        QCOMPARE(exchange(receiver,"END","0"),QByteArray("verified\n"));
        QCOMPARE(exchange(receiver,"RING","overflow"),QByteArray("verified\n"));
    }
};
QTEST_GUILESS_MAIN(MobileReceiverTest)
#include "MobileReceiverTest.moc"
