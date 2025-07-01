/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "FritzPhonebookFetcher.h"
#include <QObject>
#include <QQmlEngine>
#include <QStringList>

class KFritzCorePlugin : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit KFritzCorePlugin(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getPhonebookList(const QString &host, int port, const QString &user, const QString &password);

private:
    FritzPhonebookFetcher m_fetcher;
};