/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "PhonebookCache.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>

using namespace Qt::StringLiterals;

void PhonebookLookup::loadFromFile(const QString &xmlFilePath, const QString &countryCode)
{
    m_countryCode = countryCode;

    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "❌ Could not open phonebook XML:" << xmlFilePath;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "❌ Invalid XML in phonebook file:" << xmlFilePath;
        return;
    }

    file.close();

    QDomNodeList contacts = doc.elementsByTagName(u"contact"_s);
    for (int i = 0; i < contacts.count(); ++i) {
        QDomElement contact = contacts.at(i).toElement();
        QString name;
        QStringList numbers;

        QDomElement person = contact.firstChildElement(u"person"_s);
        if (!person.isNull()) {
            QDomElement realNameElement = person.firstChildElement(u"realName"_s);
            if (!realNameElement.isNull()) {
                name = realNameElement.text().trimmed();
            }
        }

        // if (!realName.isNull()) {
        //     name = realName.text().trimmed();
        // }

        QDomNodeList telList = contact.elementsByTagName(u"number"_s);
        for (int j = 0; j < telList.count(); ++j) {
            QDomElement numberElement = telList.at(j).toElement();
            QString raw = numberElement.text().trimmed();
            QString normalized = normalizeNumber(raw, countryCode);
            if (!normalized.isEmpty() && !name.isEmpty()) {
                numberToName.insert(normalized, name);
            }
        }
    }

    qDebug() << "✅ Phonebook loaded:" << numberToName.size() << "entries.";
}

QString PhonebookLookup::resolveName(const QString &number) const
{
    return numberToName.value(normalizeNumber(number, m_countryCode), QString{});
}

QString PhonebookLookup::normalizeNumber(QString number, const QString &countryCode) const
{
    // 1. Entferne alle Zeichen außer Ziffern und "+"
    number.remove(QRegularExpression(u"[\\s\\-\\(\\)]"_s));

    // 2. "0049..." → "+49..."
    if (number.startsWith(u"00"_s)) {
        number.replace(0, 2, u"+"_s);
    }

    // 3. Wenn mit "+" beginnt → gültig, unverändert zurückgeben
    if (number.startsWith(u"+"_s)) {
        return number;
    }

    // 4. Wenn mit 0 beginnt → lokale Nummer → ersetze führende 0 mit Landesvorwahl
    if (number.startsWith(u"0"_s)) {
        return countryCode + number.mid(1);
    }

    // 5. Sonst: keine führende 0, kein + → trotzdem lokale Nummer annehmen
    return countryCode + number;
}
