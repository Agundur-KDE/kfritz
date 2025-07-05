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
    default:
        return {};
    }
}

QHash<int, QByteArray> RecentCallsModel::roleNames() const
{
    return {{NameRole, "name"}, {NumberRole, "number"}, {TimeRole, "time"}};
}

void RecentCallsModel::addCall(const QString &name, const QString &number, const QString &time)
{
    qDebug() << "📞 RecentCallsModel::addCall:" << name << number << time;
    beginInsertRows(QModelIndex(), 0, 0);
    m_calls.prepend({name, number, time});
    if (m_calls.size() > 20) // optional: max. 20 Anrufe speichern
        m_calls.removeLast();
    endInsertRows();
}