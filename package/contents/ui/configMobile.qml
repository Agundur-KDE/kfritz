// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import de.agundur.kfritz 0.1

KCM.SimpleKCM {
    property bool cfg_MobileEnabled
    property bool cfg_MobileEnabledDefault
    property int cfg_MobilePort
    property int cfg_MobilePortDefault
    property string cfg_MobileKey
    property string cfg_MobileKeyDefault
    MobileCallReceiver { id: keyGenerator }
    Kirigami.FormLayout {
        Controls.CheckBox {
            text: i18n("Receive calls from paired phones")
            checked: cfg_MobileEnabled
            onToggled: cfg_MobileEnabled = checked
        }
        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("In ScreenRing Phone, add this computer as a destination using its local IP address, this port and this pairing key. Caller details are encrypted. Your computer must be running and reachable on the home network.")
        }
        Controls.SpinBox {
            Kirigami.FormData.label: i18n("Mobile call port:")
            from: 1; to: 65535
            value: cfg_MobilePort
            onValueModified: cfg_MobilePort = value
        }
        Controls.TextField {
            Kirigami.FormData.label: i18n("Pairing key:")
            Layout.fillWidth: true
            echoMode: TextInput.PasswordEchoOnEdit
            text: cfg_MobileKey
            onTextEdited: cfg_MobileKey = text.trim()
        }
        Controls.Button {
            text: i18n("Generate new pairing key")
            onClicked: resetPrompt.open()
        }
        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("A new key disconnects previously paired phones. Save the settings before testing the phone connection.")
        }
    }
    Kirigami.PromptDialog {
        id: resetPrompt
        title: i18n("Replace pairing key?")
        subtitle: i18n("All phones must be paired again with the new key.")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: cfg_MobileKey = keyGenerator.newPairingKey()
    }
}
