/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QString>

// TR-064 SOAP transport shared by everything that talks to the FritzBox
// (phonebook read/write, call barring, ...). Extracted out of
// FritzPhonebookFetcher, which used to be the only caller.
class FritzSOAP
{
public:
    void setHost(const QString &host);
    void setPort(int port);
    void setUsername(const QString &user);
    void setPassword(const QString &pass);

    // `ok` (if given) reports whether the request actually reached the box
    // and got a reply — distinct from the returned XML being empty, which
    // an empty-but-successful response can also produce. Callers that only
    // check the response body for an <errorCode> (as most here do) would
    // otherwise silently treat "box unreachable"/timeout the same as
    // "request succeeded, empty response".
    QString sendRequest(const QString &service, const QString &action, const QString &body, const QString &controlUrl, bool *ok = nullptr) const;

private:
    QString m_host, m_user, m_pass;
    int m_port = 49000;
};
