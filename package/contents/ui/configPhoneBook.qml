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
import de.agundur.kfritz 0.1
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kquickcontrols 2.0 as KQC
import org.kde.plasma.plasmoid

KCM.SimpleKCM {
    id: root

    property string cfg_Phonebooks
    property int cfg_Port
    property string cfg_Host
    property string cfg_Login
    property string cfg_Password
    property int cfg_CountryCode: 49
    property var cfg_ContactsPhonebooks: []
    property int cfg_ContactsWriteTarget: -1
    property var cfg_BlocklistPhonebooks: []
    property int cfg_BlocklistWriteTarget: -1

    function updatePhonebooks() {
        phonebookModel.clear();
        const books = plugin.listLocalPhonebooks();
        for (let i = 0; i < books.length; ++i) {
            phonebookModel.append({
                "id": books[i].id,
                "name": books[i].name
            });
        }
    }

    function nameForId(id) {
        for (let i = 0; i < phonebookModel.count; ++i) {
            if (phonebookModel.get(i).id === id)
                return phonebookModel.get(i).name;
        }
        return i18n("Phonebook %1", id);
    }

    function idsAsInts(stringList) {
        return stringList.map(s => parseInt(s, 10));
    }

    function addToContacts(id) {
        const ids = idsAsInts(cfg_ContactsPhonebooks);
        if (ids.includes(id))
            return;
        cfg_ContactsPhonebooks = cfg_ContactsPhonebooks.concat([String(id)]);
        if (cfg_ContactsWriteTarget === -1)
            cfg_ContactsWriteTarget = id;
    }

    function removeFromContacts(id) {
        cfg_ContactsPhonebooks = idsAsInts(cfg_ContactsPhonebooks).filter(x => x !== id).map(String);
        if (cfg_ContactsWriteTarget === id)
            cfg_ContactsWriteTarget = cfg_ContactsPhonebooks.length > 0 ? parseInt(cfg_ContactsPhonebooks[0], 10) : -1;
    }

    function addToBlocklist(id) {
        const ids = idsAsInts(cfg_BlocklistPhonebooks);
        if (ids.includes(id))
            return;
        cfg_BlocklistPhonebooks = cfg_BlocklistPhonebooks.concat([String(id)]);
        if (cfg_BlocklistWriteTarget === -1)
            cfg_BlocklistWriteTarget = id;
    }

    function removeFromBlocklist(id) {
        cfg_BlocklistPhonebooks = idsAsInts(cfg_BlocklistPhonebooks).filter(x => x !== id).map(String);
        if (cfg_BlocklistWriteTarget === id)
            cfg_BlocklistWriteTarget = cfg_BlocklistPhonebooks.length > 0 ? parseInt(cfg_BlocklistPhonebooks[0], 10) : -1;
    }

    Component.onCompleted: updatePhonebooks()

    Connections {
        function onPhonebookDownloaded(id, path) {
            updatePhonebooks();
        }

        target: plugin
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            QtControls.SpinBox {
                id: countrySpinBox

                value: cfg_CountryCode
                Kirigami.FormData.label: i18n("Country code: +")
                from: 1
                to: 999
                stepSize: 1
                textFromValue: function(value) {
                    return value;
                }
                onValueChanged: cfg_CountryCode = textFromValue(value)
            }

            QtControls.Button {
                id: getPhonebooks

                text: i18n("Get Phonebook")
                onClicked: {
                    cfg_Phonebooks = plugin.getPhonebookList(cfg_Host, cfg_Port, cfg_Login, cfg_Password).join(", ");
                }
            }

        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            Kirigami.Heading {
                level: 3
                text: i18n("Add a phonebook")
                Layout.fillWidth: true
            }

        }

        RowLayout {
            Layout.fillWidth: true

            QtControls.ComboBox {
                id: addBox

                Layout.fillWidth: true
                model: phonebookModel
                textRole: "name"
                valueRole: "id"
            }

            QtControls.Button {
                text: i18n("Add to Contacts")
                enabled: addBox.currentIndex >= 0
                onClicked: addToContacts(phonebookModel.get(addBox.currentIndex).id)
            }

            QtControls.Button {
                text: i18n("Add to Blocklist")
                enabled: addBox.currentIndex >= 0
                onClicked: addToBlocklist(phonebookModel.get(addBox.currentIndex).id)
            }

        }

        Kirigami.Heading {
            level: 3
            text: i18n("Contacts sources (used for caller ID)")
        }

        QtControls.Label {
            visible: cfg_ContactsPhonebooks.length === 0
            text: i18n("None assigned yet.")
            opacity: 0.6
        }

        Repeater {
            model: idsAsInts(cfg_ContactsPhonebooks)

            delegate: RowLayout {
                required property int modelData

                Layout.fillWidth: true

                QtControls.RadioButton {
                    text: nameForId(modelData)
                    checked: cfg_ContactsWriteTarget === modelData
                    onClicked: cfg_ContactsWriteTarget = modelData
                    Layout.fillWidth: true
                }

                QtControls.ToolTip.visible: hovered
                QtControls.ToolTip.text: i18n("Write target — new contacts are added here")

                QtControls.ToolButton {
                    icon.name: "edit-delete"
                    onClicked: removeFromContacts(modelData)
                }

            }

        }

        Kirigami.Heading {
            level: 3
            text: i18n("Blocklist sources")
        }

        QtControls.Label {
            visible: cfg_BlocklistPhonebooks.length > 0
            text: i18n("All checked phonebooks are read for blocking. The selected radio button is the single write target for \"Add to Blocklist\" — leave a synced/read-only list unselected so it never gets overwritten.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            opacity: 0.6
            font.italic: true
        }

        QtControls.Label {
            visible: cfg_BlocklistPhonebooks.length === 0
            text: i18n("None assigned yet.")
            opacity: 0.6
        }

        Repeater {
            model: idsAsInts(cfg_BlocklistPhonebooks)

            delegate: RowLayout {
                required property int modelData

                Layout.fillWidth: true

                QtControls.RadioButton {
                    text: nameForId(modelData)
                    checked: cfg_BlocklistWriteTarget === modelData
                    onClicked: cfg_BlocklistWriteTarget = modelData
                    Layout.fillWidth: true
                }

                QtControls.ToolTip.visible: hovered
                QtControls.ToolTip.text: i18n("Write target — the \"Add to Blocklist\" button on an incoming call writes here (e.g. keep a WebDAV-synced list read-only by never selecting it here)")

                QtControls.ToolButton {
                    icon.name: "edit-delete"
                    onClicked: removeFromBlocklist(modelData)
                }

            }

        }

        Item {
            Layout.fillHeight: true
        }

    }

    KFritzCorePlugin {
        id: plugin
    }

    ListModel {
        id: phonebookModel
    }

}
