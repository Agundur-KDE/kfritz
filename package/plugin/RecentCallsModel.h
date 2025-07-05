/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QHash>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>

class RecentCallsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        NumberRole,
        TimeRole
    };

    explicit RecentCallsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addCall(const QString &name, const QString &number, const QString &time);

private:
    struct CallEntry {
        QString name;
        QString number;
        QString time;
    };

    QVector<CallEntry> m_calls;
};
