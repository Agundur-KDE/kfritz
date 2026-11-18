// SPDX-License-Identifier: GPL-3.0-or-later
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <KLocalizedContext>
#include <QTextStream>
#include <memory>

int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);
    QQmlEngine engine;engine.addImportPath(QString::fromUtf8(QML_BUILD_PATH));
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QQmlComponent receiver(&engine);
    receiver.setData("import de.agundur.kfritz 0.1\nMobileCallReceiver { enabled: false }",QUrl());
    std::unique_ptr<QObject> object(receiver.create());
    if(!object) { for(const auto &e:receiver.errors())QTextStream(stderr)<<e.toString()<<Qt::endl;return 1; }
    for(const QString &file:{QStringLiteral("configMobile.qml"),QStringLiteral("main.qml")}) {
        QQmlComponent component(&engine,QUrl::fromLocalFile(QString::fromUtf8(QML_SOURCE_PATH)+QLatin1Char('/')+file));
        if(component.isError()) {for(const auto &e:component.errors())QTextStream(stderr)<<e.toString()<<Qt::endl;return 2;}
    }
    return 0;
}
