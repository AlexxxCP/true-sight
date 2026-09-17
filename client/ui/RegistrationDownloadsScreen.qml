import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import QtCore

Pane {
    id: root

    property bool shareSaved: false
    property bool privateKeySaved: false
    property string username: ""
    property string errorMessage: ""
    readonly property string suggestedName:
        username.replace(/[^A-Za-z0-9._-]/g, "_") || "user"
    signal saveShareRequested(url destination)
    signal savePrivateKeyRequested(url destination)
    signal continueRequested()

    background: Rectangle { color: "#f7f8fa" }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(520, parent.width - 80)
        spacing: 16

        Label {
            Layout.fillWidth: true
            text: "Save your keys"
            horizontalAlignment: Text.AlignHCenter
            color: "#1d2939"
            font.pixelSize: 26
            font.weight: Font.DemiBold
        }

        Label {
            Layout.fillWidth: true
            text: "Save both files before continuing. Keep the .tskey file private; share the .share file with people you want to message."
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            color: "#667085"
        }

        Basic.Button {
            id: shareButton
            Layout.fillWidth: true
            implicitHeight: 44
            text: root.shareSaved ? "Download .share file ✓" : "Download .share file"
            onClicked: {
                shareDialog.selectedFile = shareDialog.currentFolder.toString()
                    + "/" + root.suggestedName + ".share"
                shareDialog.open()
            }
        }

        Basic.Button {
            id: privateButton
            Layout.fillWidth: true
            implicitHeight: 44
            text: root.privateKeySaved ? "Download .tskey file ✓" : "Download .tskey file"
            onClicked: {
                privateDialog.selectedFile = privateDialog.currentFolder.toString()
                    + "/" + root.suggestedName + ".tskey"
                privateDialog.open()
            }
        }

        Basic.Button {
            id: continueButton
            Layout.fillWidth: true
            implicitHeight: 44
            text: "Continue"
            enabled: root.shareSaved && root.privateKeySaved

            contentItem: Text {
                text: continueButton.text
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.DemiBold
            }

            background: Rectangle {
                radius: 8
                color: !continueButton.enabled ? "#98a2b3"
                     : continueButton.down ? "#1f4f86"
                     : continueButton.hovered ? "#285f9e" : "#2f6db3"
            }

            onClicked: root.continueRequested()
        }

        Label {
            Layout.fillWidth: true
            visible: root.errorMessage.length > 0
            text: root.errorMessage
            wrapMode: Text.WordWrap
            color: "#b42318"
        }
    }

    Dialogs.FileDialog {
        id: shareDialog
        title: "Save .share file"
        fileMode: Dialogs.FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["TrueSight share files (*.share)"]
        defaultSuffix: "share"
        onAccepted: root.saveShareRequested(selectedFile)
    }

    Dialogs.FileDialog {
        id: privateDialog
        title: "Save .tskey file"
        fileMode: Dialogs.FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["TrueSight private key files (*.tskey)"]
        defaultSuffix: "tskey"
        onAccepted: root.savePrivateKeyRequested(selectedFile)
    }
}
