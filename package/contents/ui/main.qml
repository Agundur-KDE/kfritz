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
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    // Trennlinie unter dem Header
    //  ListModel {
    //     id: dummyModel
    //     ListElement {
    //         name: "Granny"
    //         number: "+49 999 1234567"
    //         time: "12:30"
    //     }
    //     ListElement {
    //         name: "El jefe"
    //         number: "+1 555-0199"
    //         time: "11:05"
    //     }
    //     ListElement {
    //         name: "ACME limited"
    //         number: "+44 7700 900000"
    //         time: "09:50"
    //     }
    // }

    id: root

    property string host: Plasmoid.configuration.Host //"192.168.178.1"
    property int port: Plasmoid.configuration.Port //49000
    property string cfg_viewMode: "fullRepresentation"
    property bool callMonitorConnected: false
    property bool showCallerInfo: false
    property bool testMode: false
    property bool pendingClearsContactName: false
    readonly property bool hasBackground: (Plasmoid.effectiveBackgroundHints & PlasmaCore.Types.DefaultBackground) !== 0

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
    function checkMissedCalls() {
        // Fire-and-forget — checkMissedCalls() now runs the SOAP call on a
        // background thread and reports back via missedCallsChecked()
        // (Connections below) instead of a direct return value, so this
        // can no longer freeze the panel.
        plugin.checkMissedCalls(Plasmoid.configuration.LastSeenCallId);
    }

    function refreshAllPhonebooks() {
        // Re-downloads every phonebook the box has (overwrites the local
        // cache), then reloads the in-memory lookup tables for the
        // currently assigned Contacts/Blocklist roles from those fresh
        // files — a stale cache silently fails to recognize numbers a
        // WebDAV-synced blocklist has picked up since the last manual
        // "Get Phonebook" click.
        plugin.getPhonebookList(Plasmoid.configuration.Host, Plasmoid.configuration.Port, Plasmoid.configuration.Login, Plasmoid.configuration.Password);
        plugin.setContactsPhonebooks(Plasmoid.configuration.ContactsPhonebooks, Plasmoid.configuration.CountryCode);
        plugin.setBlocklistPhonebooks(Plasmoid.configuration.BlocklistPhonebooks, Plasmoid.configuration.CountryCode);
    }

    Component.onCompleted: {
        plugin.setHost(Plasmoid.configuration.Host);
        plugin.setCredentials(Plasmoid.configuration.Host, Plasmoid.configuration.Port, Plasmoid.configuration.Login, Plasmoid.configuration.Password);
        plugin.connectToFritzBox();
        plugin.setContactsPhonebooks(Plasmoid.configuration.ContactsPhonebooks, Plasmoid.configuration.CountryCode);
        plugin.setBlocklistPhonebooks(Plasmoid.configuration.BlocklistPhonebooks, Plasmoid.configuration.CountryCode);
        startupMissedCallsTimer.start();
        if (Plasmoid.configuration.AutoSyncPhonebooks)
            autoSyncStartupTimer.start();
    }

    onExpandedChanged: if (expanded)
        plugin.clearMissedBadge()

    Timer {
        // checkMissedCalls() blocks on a SOAP call — give the network a
        // moment at login/plasmashell start instead of hitting it from
        // Component.onCompleted, matching FritzCallMonitor's own 5s delay.
        id: startupMissedCallsTimer
        interval: 5000
        onTriggered: checkMissedCalls()
    }

    Timer {
        // Offset from startupMissedCallsTimer so the two blocking network
        // sequences don't overlap on login.
        id: autoSyncStartupTimer
        interval: 10000
        onTriggered: refreshAllPhonebooks()
    }

    Timer {
        id: autoSyncRecurringTimer
        interval: Plasmoid.configuration.AutoSyncIntervalDays * 24 * 60 * 60 * 1000
        running: Plasmoid.configuration.AutoSyncPhonebooks
        repeat: true
        onTriggered: refreshAllPhonebooks()
    }

    KFritzCorePlugin {
        id: plugin
    }

    Connections {
        function onCurrentCallerChanged() {
            showCallerInfo = true;
            hideTimer.restart();
        }

        function onMissedCallsChecked(newLastSeenId) {
            Plasmoid.configuration.LastSeenCallId = newLastSeenId;
        }

        function onAddPhonebookEntryFinished(ok) {
            addEntryError.visible = !ok;
            if (ok && pendingClearsContactName) {
                newContactName.text = "";
            }
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

        background: null
        padding: 0
        Kirigami.Theme.inherit: true
        // Optionaler Seitentitel (erscheint z.B. in der mobilen Toolbar)
        title: i18n("Recent Calls")

        ListView {
            id: callsList

            clip: true
            model: plugin.recentCallsModel

            // flacher Delegate ohne Hintergrund
            delegate: Controls.ItemDelegate {
                required property string name
                required property string number
                required property string time
                required property bool blocked

                // Kein Hintergrund zeichnen:
                background: null
                hoverEnabled: true
                padding: Kirigami.Units.smallSpacing

                Kirigami.Separator {
                    visible: hasBackground
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    // Blocked is also conveyed via icon + strikethrough, not
                    // color alone.
                    Kirigami.Icon {
                        source: blocked ? "call-stop" : "user"
                        // isMask: true is required for `color` to actually
                        // take effect — without it Kirigami.Icon silently
                        // ignores the color property on non-auto-detected
                        // icons and just renders them in their own colors.
                        isMask: true
                        color: blocked ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Two-line title/subtitle instead of one RichText line
                    // built from string concatenation — avoids caller-name
                    // data being interpreted as HTML, and reads more clearly
                    // than everything crammed onto one line.
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Controls.Label {
                            // Falls back to the number when there's no
                            // resolved name, so the title line is never
                            // empty.
                            text: blocked ? number : (name.length > 0 ? name : number)
                            font.bold: true
                            font.strikeout: blocked
                            color: blocked ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Controls.Label {
                            visible: !blocked
                            text: number + " – " + time
                            color: Kirigami.Theme.subtitleColor
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                    }

                }

            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                // Model exists as soon as the plugin is instantiated, so an
                // empty list here genuinely means "no calls yet" rather than
                // "still loading" — there's no separate loading state to
                // race against.
                visible: callsList.count === 0
                icon.name: "call-start"
                text: i18n("No calls yet")
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
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    text: "FRITZ!Box"
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 10
                    // Verified via plasmoidviewer: Kirigami.Theme.subtitleColor
                    // resolves to undefined on a plain Controls.Label here,
                    // and Qt silently defaults an undefined color assignment
                    // to black — invisible against a dark panel. The
                    // fallback isn't dead defensive code, it's load-bearing.
                    color: Kirigami.Theme.subtitleColor ?? "#666666"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    // Color alone isn't accessible (colorblind users can't
                    // tell red from green) — icon shape + tooltip text carry
                    // the same information redundantly.
                    Kirigami.Icon {
                        // implicitWidth/Height are only a Layout sizing
                        // hint — the icon's actual rendered pixmap ignored
                        // them and rendered oversized/clipped instead of
                        // scaled. width/height controls the real render size.
                        width: Kirigami.Units.gridUnit * 0.8
                        height: Kirigami.Units.gridUnit * 0.8
                        source: plugin.callMonitorConnected ? "network-connect-symbolic" : "network-disconnect-symbolic"
                        // isMask: true is required for `color` to actually
                        // take effect — confirmed via qmlplugindump
                        // (Kirigami.Icon has both isMask and color
                        // properties; color is silently ignored without it).
                        // The plain -symbolic icon alone rendered its own
                        // tiny, low-contrast default color, which is what
                        // looked like "black on black" during testing.
                        isMask: true
                        color: plugin.callMonitorConnected ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                        Layout.alignment: Qt.AlignVCenter

                        Controls.ToolTip.visible: statusIconHover.hovered
                        Controls.ToolTip.text: plugin.callMonitorConnected ? i18n("Call Monitor connected") : i18n("Call Monitor disconnected")

                        HoverHandler {
                            id: statusIconHover
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Controls.ToolButton {
                        icon.name: "view-refresh"
                        text: i18n("Check for missed calls")
                        display: Controls.ToolButton.IconOnly
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: checkMissedCalls()

                        Controls.ToolTip.visible: hovered
                        Controls.ToolTip.text: text
                    }

                    HelpTipButton {
                        helpText: i18n("Dial: #96*5* to enable the CallMonitor on your Fritz!Box")
                        Layout.alignment: Qt.AlignVCenter
                    }

                }

            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: showCallerInfo && plugin.callerUnknown

                Controls.Label {
                    text: i18n("Unknown number: %1", plugin.currentCallerNumber)
                }

                RowLayout {
                    Layout.fillWidth: true

                    Controls.TextField {
                        id: newContactName

                        placeholderText: i18n("Name")
                        Layout.fillWidth: true
                    }

                    Controls.ComboBox {
                        id: newContactType

                        property var typeValues: ["home", "mobile", "work"]

                        model: [i18n("Private"), i18n("Mobile"), i18n("Business")]
                    }

                    Controls.Button {
                        text: i18n("Add to Contacts")
                        enabled: Plasmoid.configuration.ContactsWriteTarget !== -1 && newContactName.text.length > 0
                        onClicked: {
                            // addPhonebookEntry() runs on a background thread
                            // now — result arrives later via
                            // addPhonebookEntryFinished (Connections below).
                            // Remember that this click was the one clearing
                            // the name field on success (Add-to-Blocklist
                            // doesn't touch it).
                            pendingClearsContactName = true;
                            plugin.addPhonebookEntry(Plasmoid.configuration.ContactsWriteTarget, newContactName.text, plugin.currentCallerNumber, newContactType.typeValues[newContactType.currentIndex]);
                        }
                    }

                    Controls.Button {
                        text: i18n("Add to Blocklist")
                        enabled: Plasmoid.configuration.BlocklistWriteTarget !== -1
                        onClicked: {
                            pendingClearsContactName = false;
                            plugin.addPhonebookEntry(Plasmoid.configuration.BlocklistWriteTarget, i18n("Blocked"), plugin.currentCallerNumber, "home");
                        }
                    }

                }

                Kirigami.InlineMessage {
                    id: addEntryError
                    Layout.fillWidth: true
                    visible: false
                    type: Kirigami.MessageType.Error
                    text: i18n("Could not reach the FritzBox — entry was not added. Check your connection and try again.")
                    showCloseButton: true
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

        Rectangle {
            visible: plugin.missedCount > 0
            width: Math.max(badgeLabel.implicitWidth + Kirigami.Units.smallSpacing, Kirigami.Units.gridUnit)
            height: Kirigami.Units.gridUnit
            radius: height / 2
            color: Kirigami.Theme.negativeBackgroundColor
            anchors.top: parent.top
            anchors.right: parent.right

            Controls.Label {
                id: badgeLabel

                anchors.centerIn: parent
                text: plugin.missedCount > 99 ? "99+" : plugin.missedCount
                // highlightedTextColor rather than negativeTextColor: the
                // latter is a signal color for text-on-normal-background,
                // not guaranteed to contrast against negativeBackgroundColor
                // itself.
                color: Kirigami.Theme.highlightedTextColor
                font.pixelSize: Kirigami.Units.gridUnit * 0.6
            }

        }

    }

}
