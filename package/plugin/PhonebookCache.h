/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QHash>
#include <QList>
#include <QPair>
#include <QString>

class PhonebookLookup
{
public:
    void loadFromFile(const QString &xmlFilePath, const QString &countryCode);
    QString resolveName(const QString &number) const;

private:
    QHash<QString, QString> numberToName;
    // AVM's own wildcard syntax: a trailing '*' on a phonebook number means
    // "this prefix and everything after it" — common in bulk spam lists
    // (one entry can cover thousands of numbers). Stored separately since a
    // plain QHash can only do exact matches.
    QList<QPair<QString, QString>> wildcardPrefixes;
    QString m_countryCode;

    QString normalizeNumber(QString number, const QString &countryCode) const;
};
