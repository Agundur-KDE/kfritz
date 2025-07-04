/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QHash>
#include <QString>

class PhonebookLookup
{
public:
    void loadFromFile(const QString &xmlFilePath, const QString &countryCode);
    QString resolveName(const QString &number) const;

private:
    QHash<QString, QString> numberToName;
    QString m_countryCode;

    QString normalizeNumber(QString number, const QString &countryCode) const;
};
