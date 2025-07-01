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
    property int cfg_Port
    property string cfg_Host
    property string cfg_HostDefault
    property int cfg_PortDefault

    Kirigami.FormLayout {
        QtControls.TextField {
            id: ipTextField

            inputMask: "000.000.000.000"
            readOnly: false
            Layout.fillWidth: true
            Kirigami.FormData.label: "EZ1 IP:"
            text: cfg_Host
            onEditingFinished: cfg_Host = text
        }

        QtControls.SpinBox {
            id: portSpinBox

            value: cfg_Port
            Kirigami.FormData.label: "EZ1 Port:"
            from: 1
            to: 65535
            stepSize: 1
            textFromValue: function(value) {
                return value;
            }
            onValueChanged: cfg_Port = textFromValue(value)
        }

    }

}