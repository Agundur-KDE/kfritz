import QtQuick 6.5
import QtQuick.Controls 6.5 as Controls
import org.kde.kirigami 2.0 as Kirigami

Controls.ToolButton {
    id: control

    property string helpText: ""

    focusPolicy: Qt.TabFocus
    padding: 0
    implicitWidth: Math.round(Kirigami.Units.gridUnit * 0.9)
    implicitHeight: implicitWidth
    Controls.ToolTip.text: control.helpText
    Controls.ToolTip.delay: 250
    Controls.ToolTip.visible: control.hovered || control.activeFocus
    Accessible.role: Accessible.Button
    Accessible.name: qsTr("Help")
    Accessible.description: control.helpText

    // Cursor auf "Hand" setzen
    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        radius: height / 2
        antialiasing: true
        color: control.hovered || control.activeFocus ? Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.18) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.12)
        border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.35)
        border.width: 1
    }

    contentItem: Text {
        text: "?"
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: Kirigami.Theme.textColor
    }

}
