// SPDX-License-Identifier: GPL-3.0-or-later
#include "MobileCallReceiver.h"
#include "CallOsd.h"
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QSaveFile>
#include <KLocalizedString>
#include <KNotification>
#include <QTextStream>

int main(int argc,char **argv) {
    if (!qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) qputenv("QT_QPA_PLATFORM","wayland");
    QGuiApplication app(argc,argv);app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setApplicationName(QStringLiteral("kfritz-mobile-probe"));
    KLocalizedString::setApplicationDomain("plasma_applet_de.agundur.kfritz");
    QCommandLineParser parser;parser.addHelpOption();
    parser.addOption({QStringLiteral("key-file"),QStringLiteral("Private pairing key file"),QStringLiteral("path")});
    parser.addOption({QStringLiteral("port"),QStringLiteral("TCP port"),QStringLiteral("port"),QStringLiteral("10123")});
    parser.process(app);
    const QString path=parser.value(QStringLiteral("key-file"));if(path.isEmpty())return 2;
    MobileCallReceiver receiver;
    QFile input(path);QString key;
    if(input.open(QIODevice::ReadOnly)) key=QString::fromLatin1(input.readAll().trimmed());
    if(!MobileProtocol::validKey(key)) {
        key=receiver.newPairingKey();QSaveFile output(path);
        if(!output.open(QIODevice::WriteOnly))return 3;
        output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);output.write(key.toLatin1());if(!output.commit())return 3;
    }
    CallOsd osd;QString visible;
    auto show=[&](const QString &name,const QString &number,const QString &label) {
        osd.showCall(name.isEmpty()?label:name,number,QFont(QStringLiteral("Sans Serif"),18),2,QColor(Qt::white));
    };
    QObject::connect(&receiver,&MobileCallReceiver::incomingCall,&app,[&](const QString &id,const QString &name,const QString &number,const QString &label) {
        visible=id;show(name,number,label);
        auto *n=new KNotification(QStringLiteral("callReceived"),KNotification::CloseOnTimeout);n->setComponentName(QStringLiteral("kfritz"));n->setTitle(i18n("Mobile call · %1",label));
        n->setText(name.isEmpty()?label.toHtmlEscaped():name.toHtmlEscaped());n->setIconName(QStringLiteral("call-incoming"));n->sendEvent();
        QTextStream(stdout)<<"RING name_present="<<!name.isEmpty()<<" number_present="<<!number.isEmpty()<<Qt::endl;
    });
    QObject::connect(&receiver,&MobileCallReceiver::callerUpdated,&app,[&](const QString &id,const QString &name,const QString &number,const QString &label) {
        if(visible==id)show(name,number,label);
        QTextStream(stdout)<<"UPDATE name_present="<<!name.isEmpty()<<" number_present="<<!number.isEmpty()<<Qt::endl;
    });
    QObject::connect(&receiver,&MobileCallReceiver::testMessage,&app,[&](const QString &label) {
        show(i18n("Phone connected"),{},label);QTextStream(stdout)<<"TEST accepted"<<Qt::endl;
    });
    QObject::connect(&receiver,&MobileCallReceiver::callEnded,&app,[&](const QString &id) {if(visible==id){osd.hide();visible.clear();}QTextStream(stdout)<<"END accepted"<<Qt::endl;});
    receiver.setPairingKey(key);receiver.setPort(parser.value(QStringLiteral("port")).toInt());receiver.setEnabled(true);
    if(!receiver.listening())return 4;
    QTextStream(stdout)<<"READY port="<<receiver.port()<<" platform="<<QGuiApplication::platformName()<<" osd_supported="<<osd.supported()<<Qt::endl;
    return app.exec();
}
