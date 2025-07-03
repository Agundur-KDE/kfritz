/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "FritzCallMonitor.h"
#include "FritzPhonebookFetcher.h"
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

class KFritzCorePlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *callMonitor READ callMonitor CONSTANT)
    QML_ELEMENT

public:
    explicit KFritzCorePlugin(QObject *parent = nullptr);
    QObject *callMonitor();

    Q_INVOKABLE QVariantList getPhonebookList(const QString &host, int port, const QString &user, const QString &password);
    Q_INVOKABLE QVariantList listLocalPhonebooks();
    Q_INVOKABLE void connectToFritzBox();

public Q_SLOTS:
    // void connectToFritzBox();

Q_SIGNALS:
    void phonebookDownloaded(int id, const QString &path);

private:
    FritzPhonebookFetcher m_fetcher;
    FritzCallMonitor m_callMonitor;
};