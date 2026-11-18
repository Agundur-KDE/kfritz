// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "MobileProtocol.h"
#include <QObject>
#include <QQmlEngine>
#include <QTcpServer>
#include <QHash>
#include <QElapsedTimer>
#include <QTimer>

class MobileCallReceiver : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY changed)
    Q_PROPERTY(QString pairingKey READ pairingKey WRITE setPairingKey NOTIFY changed)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY changed)
    Q_PROPERTY(bool listening READ listening NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
public:
    explicit MobileCallReceiver(QObject *parent=nullptr);
    bool enabled() const { return m_enabled; }
    QString pairingKey() const { return m_key; }
    int port() const { return m_port; }
    bool listening() const { return m_server.isListening(); }
    QString status() const;
    void setEnabled(bool value);
    void setPairingKey(const QString &value);
    void setPort(int value);
    Q_INVOKABLE QString newPairingKey() const;
Q_SIGNALS:
    void changed();
    void incomingCall(const QString &id,const QString &name,const QString &number,const QString &label);
    void callerUpdated(const QString &id,const QString &name,const QString &number,const QString &label);
    void callEnded(const QString &id);
    void testMessage(const QString &label);
private:
    struct Call { MobileMessage message; qint64 seen; bool answered=false; };
    void restart();
    void accept();
    void process(const MobileMessage &message);
    bool m_enabled=false;
    QString m_key;
    int m_port=10123;
    QString m_status=QStringLiteral("disabled");
    QTcpServer m_server;
    QElapsedTimer m_clock;
    QTimer m_expiry;
    QHash<QString,Call> m_calls;
    int m_clients=0;
};
