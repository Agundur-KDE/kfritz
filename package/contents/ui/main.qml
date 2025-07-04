/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

import QtQuick
import QtQuick.Controls 6.5
import QtQuick.Layouts
import de.agundur.kfritz 0.1
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    property string host: Plasmoid.configuration.Host //"192.168.178.1"
    property int port: Plasmoid.configuration.Port //49000
    property string cfg_viewMode: "fullRepresentation"
    property bool callMonitorConnected: false // später dynamisch setzen

    toolTipMainText: Plasmoid.title
    preferredRepresentation: {
        if (Plasmoid.location === PlasmaCore.Types.Floating || Plasmoid.location === PlasmaCore.Types.Desktop)
            return cfg_viewMode === "Compact" ? compactRepresentation : fullRepresentation;

        return compactRepresentation;
    }
    Plasmoid.icon: "call-incoming"
    Plasmoid.status: PlasmaCore.Types.ActiveStatus
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
    Layout.fillWidth: true
    Layout.fillHeight: true
    width: 400
    height: 300
    Component.onCompleted: {
        plugin.setHost(Plasmoid.configuration.Host);
        plugin.connectToFritzBox();
        plugin.loadPhonebook(Plasmoid.configuration.SelectedPhonebook, Plasmoid.configuration.CountryCode);
    }

    KFritzCorePlugin {
        id: plugin
    }

    fullRepresentation: Item {
        id: fullView

        Layout.preferredWidth: 400
        Layout.preferredHeight: 300

        ColumnLayout {
            // anchors.top: parent.top
            // Button {
            //     text: "recentCalls testen"
            //     onClicked: {
            //         console.log("🔍 Aktuelle recentCalls:", plugin.recentCalls);
            //         console.log("📏 Anzahl:", plugin.recentCalls.length);
            //     }
            // }

            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                // horizontalAlignment: Text.AlignHCenter

                text: "KFritz"
                level: 2
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true

                PlasmaComponents.Label {
                    id: fritzIp

                    font.pointSize: 8
                    color: "lightgray"
                    horizontalAlignment: Text.AlignLeft
                    text: Plasmoid.configuration.Host
                }

                Rectangle {
                    id: ledIndicator

                    width: 10
                    height: 10
                    radius: 5 // macht es rund
                    color: plugin.callMonitorConnected ? "green" : "red"
                    border.color: "grey"
                    border.width: 1
                    Layout.alignment: Qt.AlignLeft
                }

            }

            Rectangle {
                width: parent.width
                height: parent.height * 0.8

                Text {
                    id: callerText

                    anchors.centerIn: parent
                    text: plugin.resolveName(plugin.currentCaller) + " ( " + plugin.currentCaller + " )"
                    font.pixelSize: 16
                    color: "green"
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: plugin.currentCaller
                }

                ListView {
                    // model: ["09:15 – Test A", "10:22 – Test B"]

                    anchors.fill: parent
                    clip: true
                    model: plugin.recentCalls

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: Kirigami.Units.gridUnit * 1.2
                        border.color: "grey"
                        radius: Kirigami.Units.smallSpacing

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: "black"
                        }

                    }

                }

            }

        }

    }

    compactRepresentation: Item {
        id: compactView

        Layout.minimumWidth: iconItem.implicitWidth
        Layout.minimumHeight: iconItem.implicitHeight

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                expanded = !expanded;
            }
            cursorShape: Qt.PointingHandCursor
        }

        RowLayout {
            Kirigami.Icon {
                id: iconItem

                source: "call-incoming"
                implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
                implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
            }

            Rectangle {
                id: ledIndicator

                width: 10
                height: 10
                radius: 5 // macht es rund
                color: plugin.callMonitorConnected ? "green" : "red"
                border.color: "white"
                border.width: 1
            }

        }

    }

}
