#include "RecentCallsModel.h"

RecentCallsModel::RecentCallsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RecentCallsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_calls.size();
}
QVariant RecentCallsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_calls.size())
        return {};

    const CallEntry &call = m_calls.at(index.row());

    switch (role) {
    case NameRole:
        return call.name;
    case NumberRole:
        return call.number;
    case TimeRole:
        return call.time;
    case SourceRole:
        return call.source;
    case BlockedRole:
        return call.blocked;
    default:
        return {};
    }
}

QHash<int, QByteArray> RecentCallsModel::roleNames() const
{
    return {{NameRole, "name"}, {NumberRole, "number"}, {TimeRole, "time"}, {SourceRole, "callSource"}, {BlockedRole, "blocked"}};
}

void RecentCallsModel::clearAll()
{
    if (m_calls.isEmpty())
        return;

    beginResetModel();
    m_calls.clear();
    endResetModel();
}

void RecentCallsModel::addCall(const QString &name, const QString &number, const QString &time, bool blocked, const QString &id, const QString &source)
{
    // Never put caller names or numbers in diagnostic logs.

    beginInsertRows(QModelIndex(), 0, 0);
    m_calls.prepend({name, number, time, blocked, id, source});
    endInsertRows();

    // Verlauf begrenzen — als eigene, korrekt signalisierte Entfernung statt
    // stillschweigend innerhalb von beginInsertRows()/endInsertRows() zu
    // löschen (das verletzt das Qt-Model-Protokoll und kann Views/Delegates
    // in einen inkonsistenten Zustand bringen).
    static constexpr int maxEntries = 20;
    if (m_calls.size() > maxEntries) {
        const int lastRow = m_calls.size() - 1;
        beginRemoveRows(QModelIndex(), lastRow, lastRow);
        m_calls.removeLast();
        endRemoveRows();
    }
}

void RecentCallsModel::updateCall(const QString &id, const QString &name, const QString &number, bool blocked)
{
    if (id.isEmpty()) return;
    for (int row=0;row<m_calls.size();++row) if (m_calls[row].id==id) {
        m_calls[row].name=name; m_calls[row].number=number; m_calls[row].blocked=blocked;
        Q_EMIT dataChanged(index(row),index(row),{NameRole,NumberRole,BlockedRole}); return;
    }
}
