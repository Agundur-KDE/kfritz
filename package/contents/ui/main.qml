/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

import QtQuick
import QtQuick.Controls 2.15
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

    toolTipMainText: Plasmoid.title
    preferredRepresentation: {
        if (Plasmoid.location === PlasmaCore.Types.Floating || Plasmoid.location === PlasmaCore.Types.Desktop)
            return cfg_viewMode === "Compact" ? compactRepresentation : fullRepresentation;

        return compactRepresentation;
    }
    Plasmoid.icon: "call-incomming"
    Plasmoid.status: PlasmaCore.Types.ActiveStatus
    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    KFritzCorePlugin {
        id: plugin
    }

    fullRepresentation: Item {
        id: fullView

        Layout.minimumWidth: fritzIp.implicitWidth + 200
        Layout.minimumHeight: logoWrapper.implicitHeight + 200
        implicitWidth: 320
        implicitHeight: 300

        ColumnLayout {
            anchors.fill: parent
            // spacing: 12
            anchors.margins: Kirigami.Units.largeSpacing

            RowLayout {
                Item {
                    id: logoWrapper

                    anchors.left: parent.left
                    anchors.top: parent.top
                    width: 64
                    height: 64
                    // ToolTip.visible: kcastIcon.containsMouse
                    ToolTip.delay: 500
                    ToolTip.text: "KFritz"

                    Image {
                        id: kfritzIcon

                        source: Qt.resolvedUrl("../icons/kfritz_icon_64x64.png")
                        width: 64
                        height: 64
                        fillMode: Image.PreserveAspectFit
                    }

                }

                Kirigami.Heading {
                    text: "KCFritz"
                    level: 2
                    Layout.fillWidth: true
                }

            }

            RowLayout {
                id: fritzIp

                PlasmaComponents.Label {
                    text: Plasmoid.configuration.Host
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
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

        Kirigami.Icon {
            id: iconItem

            source: "call-incomming"
            implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
            implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
        }

    }

}
