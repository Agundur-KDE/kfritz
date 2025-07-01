/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

import QtCore
import QtQuick 2.15
import QtQuick.Controls 2.15 as QtControls
import QtQuick.Dialogs as QtDialogs
import QtQuick.Layouts 1.15
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kquickcontrols 2.0 as KQC

KCM.SimpleKCM {
    property string cfg_Phonebooks

    Kirigami.FormLayout {
        QtControls.Button {
            id: getPhonebooks

            text: "Get Phonebook"
            onClicked: {
                cfg_Phonebooks = plugin.fetchPhonebookList(cfg_Host, cfg_Port, cfg_Login, cfg_Password).join(", ");
            }
        }

    }

}