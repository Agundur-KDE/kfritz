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
    property int cfg_PortDefault
    property string cfg_Host
    property string cfg_HostDefault
    property string cfg_Login
    property string cfg_LoginDefault
    property string cfg_Password
    property string cfg_PasswordDefault

    Kirigami.FormLayout {
        QtControls.TextField {
            id: ipTextField

            inputMask: "000.000.000.000"
            readOnly: false
            Layout.fillWidth: true
            Kirigami.FormData.label: "FritzBox IP:"
            text: cfg_Host
            onEditingFinished: cfg_Host = text
        }

        QtControls.SpinBox {
            id: portSpinBox

            value: cfg_Port
            Kirigami.FormData.label: "FritzBox Port:"
            from: 1
            to: 65535
            stepSize: 1
            textFromValue: function(value) {
                return value;
            }
            onValueChanged: cfg_Port = textFromValue(value)
        }

        QtControls.TextField {
            id: loginTextField

            readOnly: false
            Layout.fillWidth: true
            Kirigami.FormData.label: "FritzBox user:"
            text: cfg_Login
            onEditingFinished: cfg_Login = text
        }

        QtControls.TextField {
            id: passwordTextField

            readOnly: false
            Layout.fillWidth: true
            Kirigami.FormData.label: "FritzBox password:"
            text: cfg_Password
            onEditingFinished: cfg_Password = text
        }

    }

}