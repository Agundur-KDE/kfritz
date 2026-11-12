// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: card
    required property string callerName
    required property string callerNumber
    required property string heading
    required property font callerFont
    required property color textColor

    width: Math.min(640, Math.max(340, content.implicitWidth + 48))
    height: content.implicitHeight + 40
    radius: 16
    color: "#ee18232e"
    border.color: "#607c96a8"
    border.width: 1

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 20
        spacing: 18

        Kirigami.Icon {
            source: "call-incoming"
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                text: card.heading
                color: card.textColor
                font.family: card.callerFont.family
                font.pointSize: 11
                textFormat: Text.PlainText
            }
            Text {
                Layout.fillWidth: true
                text: card.callerName
                font: card.callerFont
                color: card.textColor
                textFormat: Text.PlainText
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: card.callerNumber
                font: card.callerFont
                color: card.textColor
                textFormat: Text.PlainText
                elide: Text.ElideRight
            }
        }
    }
}
