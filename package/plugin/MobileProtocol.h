// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QByteArray>
#include <QString>
#include <optional>

struct MobileMessage {
    QString device, event, call, label, name, number;
};
namespace MobileProtocol {
bool validKey(const QString &key);
QByteArray newChallenge();
std::optional<MobileMessage> verify(const QString &key, const QByteArray &challenge, const QByteArray &line);
}
