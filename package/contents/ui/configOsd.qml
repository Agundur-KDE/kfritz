// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import de.agundur.kfritz 0.1

KCM.SimpleKCM {
    id: page
    property bool cfg_OsdEnabled
    property bool cfg_OsdEnabledDefault
    property font cfg_OsdFont
    property font cfg_OsdFontDefault
    property color cfg_OsdTextColor
    property color cfg_OsdTextColorDefault
    property int cfg_OsdPosition
    property int cfg_OsdPositionDefault

    CallOsd { id: preview }

    Dialogs.FontDialog {
        id: fontDialog
        title: i18n("Choose OSD font")
        onAccepted: page.cfg_OsdFont = selectedFont
    }

    Dialogs.ColorDialog {
        id: colorDialog
        title: i18n("Choose OSD text color")
        onAccepted: page.cfg_OsdTextColor = selectedColor
    }

    Kirigami.FormLayout {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !preview.supported
            type: Kirigami.MessageType.Warning
            text: i18n("The call overlay requires a Plasma Wayland session.")
        }

        Controls.CheckBox {
            Kirigami.FormData.label: i18n("Call OSD:")
            text: i18n("Show incoming calls over fullscreen applications")
            enabled: preview.supported
            checked: page.cfg_OsdEnabled
            onToggled: page.cfg_OsdEnabled = checked
        }

        Controls.Button {
            Kirigami.FormData.label: i18n("Font:")
            enabled: preview.supported
            text: page.cfg_OsdFont.family + " · " + page.cfg_OsdFont.pointSize + " pt"
            onClicked: {
                fontDialog.selectedFont = page.cfg_OsdFont;
                fontDialog.open();
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Text color:")
            enabled: preview.supported
            Rectangle {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 4
                color: page.cfg_OsdTextColor
                border.width: 1
                border.color: Kirigami.Theme.textColor
            }
            Controls.Button {
                text: i18n("Choose color…")
                onClicked: {
                    colorDialog.selectedColor = page.cfg_OsdTextColor;
                    colorDialog.open();
                }
            }
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Position:")
            enabled: preview.supported
            model: [i18n("Top left"), i18n("Top center"), i18n("Top right"),
                    i18n("Middle left"), i18n("Center"), i18n("Middle right"),
                    i18n("Bottom left"), i18n("Bottom center"), i18n("Bottom right")]
            currentIndex: page.cfg_OsdPosition
            onActivated: page.cfg_OsdPosition = currentIndex
        }

        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("The OSD appears for 8 seconds on the active screen without taking focus. Standard notifications and sounds remain enabled. Blocked calls stay silent.")
        }

        Controls.Button {
            enabled: preview.supported
            text: i18n("Test OSD")
            icon.name: "view-preview"
            onClicked: preview.showCall(i18n("Max Mustermann"), "+49 30 1234567",
                                       page.cfg_OsdFont, page.cfg_OsdPosition, page.cfg_OsdTextColor)
        }
    }
}
