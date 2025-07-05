/*
 * SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

import QtQuick
import QtQuick.Controls 6.5 as Controls
import QtQuick.Layouts
import de.agundur.kfritz 0.1
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.components 1.0 as KirigamiAddons
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    // Trennlinie unter dem Header

    id: root

    property string host: Plasmoid.configuration.Host //"192.168.178.1"
    property int port: Plasmoid.configuration.Port //49000
    property string cfg_viewMode: "fullRepresentation"
    property bool callMonitorConnected: false
    property bool showCallerInfo: false

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

    Connections {
        function onCurrentCallerChanged() {
            showCallerInfo = true;
            hideTimer.restart();
        }

        target: plugin
    }

    Timer {
        id: hideTimer

        interval: 10000 // 10 Sekunden
        running: false
        repeat: false
        onTriggered: showCallerInfo = false
    }

    fullRepresentation: Kirigami.ScrollablePage {
        id: fullPage

        // Optionaler Seitentitel (erscheint z.B. in der mobilen Toolbar)
        title: i18n("Recent Calls")

        // ListView der letzten Anrufe
        ListView {
            // Optional: mit KirigamiAddons.Avatar ein echtes Initialen-Icon einbinden:
            // contentItem: RowLayout {
            //     KirigamiAddons.Avatar {
            //         name: name
            //         size: Kirigami.Units.iconSizes.medium
            //     }
            //     Controls.Label {
            //         text: qsTr("%1 (%2)\n%3").arg(name, number, time)
            //         verticalAlignment: Text.AlignVCenter
            //         wrapMode: Text.WordWrap
            //     }
            // }

            id: callsList

            model: plugin.recentCallsModel // Datenmodell (z.B. ein ListModel oder QStringList vom Plugin)

            // Delegate-Komponente für jeden Listeneintrag
            delegate: Kirigami.InlineMessage {
                required property string name
                required property string number
                required property string time

                Layout.fillWidth: true
                visible: true
                type: Kirigami.MessageType.Information
                icon.source: "user"
                text: "<b>" + name + " </b> " + number + " – " + time
            }

        }

        // Header-Bereich oben: Überschrift + Statuszeile (aus Kirigami Gallery übernommenes Muster)
        header: ColumnLayout {
            Kirigami.Heading {
                level: 2
                text: i18n("Recent Calls") // Überschrift der Seite
                Layout.alignment: Qt.AlignHCenter // zentriert
            }

            RowLayout {
                Layout.fillWidth: true

                Controls.Label {
                    text: Plasmoid.configuration.Host
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 10
                    color: Kirigami.Theme.subtitleColor ?? "#666666"
                }

                Rectangle {
                    width: Kirigami.Units.gridUnit
                    height: Kirigami.Units.gridUnit
                    radius: Kirigami.Units.gridUnit / 2
                    color: plugin.callMonitorConnected ? "green" : "red"
                }

            }

            Kirigami.Separator {
            }

        }

    }

    compactRepresentation: Item {
        id: compactView

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

            source: "call-incoming"
            anchors.fill: parent
        }

    }

}
