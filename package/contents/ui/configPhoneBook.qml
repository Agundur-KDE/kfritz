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
    property string cfg_Phonebooks
    property int cfg_Port
    property string cfg_Host
    property string cfg_Login
    property string cfg_Password
    property int cfg_CountryCode: 49
    property int cfg_SelectedPhonebook: 0

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

    Component.onCompleted: updatePhonebooks()

    Connections {
        function onPhonebookDownloaded(id, path) {
            updatePhonebooks();
        }

        target: plugin
    }

    Kirigami.FormLayout {
        anchors.margins: Kirigami.Units.largeSpacing

        QtControls.SpinBox {
            id: countrySpinBox

            value: cfg_CountryCode
            Kirigami.FormData.label: "Country code: +"
            from: 1
            to: 999
            stepSize: 1
            textFromValue: function(value) {
                return value;
            }
            onValueChanged: cfg_CountryCode = textFromValue(value)
        }

        Item {
            Kirigami.FormData.label: "" // leere Spalte 1
            implicitHeight: Kirigami.Units.largeSpacing
        }

        Kirigami.Separator {
            // implicitHeight: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        Item {
            Kirigami.FormData.label: "" // leere Spalte 1
            implicitHeight: Kirigami.Units.largeSpacing
        }

        QtControls.Button {
            id: getPhonebooks

            text: "Get Phonebook"
            onClicked: {
                cfg_Phonebooks = plugin.getPhonebookList(cfg_Host, cfg_Port, cfg_Login, cfg_Password).join(", ");
            }
            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        QtControls.ComboBox {
            model: phonebookModel
            textRole: "name"
            valueRole: "id"
            Kirigami.FormData.label: "Phonebook:"
            currentIndex: {
                for (var i = 0; i < phonebookModel.count; ++i) {
                    if (phonebookModel.get(i).id === cfg_SelectedPhonebook)
                        return i;

                }
                return 0;
            }
            onActivated: {
                cfg_SelectedPhonebook = phonebookModel.get(currentIndex).id;
            }
        }

    }

    KFritzCorePlugin {
        id: plugin
    }

    ListModel {
        id: phonebookModel
    }

}