/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

import KFritzCorePlugin
import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
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

        Layout.minimumWidth: 300
        Layout.minimumHeight: 200

        Component {
            id: windowIconComponent

            Kirigami.Icon {
                source: "call-incomming"
                width: 22
                height: 22
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
