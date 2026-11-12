// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QColor>
#include <QFont>
#include <QObject>
#include <QQmlEngine>
#include <QQuickView>
#include <QTimer>
#include <memory>

class CallOsd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool supported READ supported CONSTANT)

public:
    explicit CallOsd(QObject *parent = nullptr);
    bool supported() const;
    Q_INVOKABLE void showCall(const QString &name, const QString &number, const QFont &font, int position, const QColor &textColor);
    Q_INVOKABLE void hide();

private:
    std::unique_ptr<QQuickView> m_view;
    QTimer m_timeout;
};
