import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs

Rectangle {
    id: root

    required property string title
    property url selectedFile
    property string acceptedSuffix: ""

    readonly property bool hasSelection: selectedFile.toString().length > 0

    implicitHeight: 126
    radius: 10
    color: dropArea.containsDrag ? "#eff6ff" : "#ffffff"
    border.width: 1
    border.color: dropArea.containsDrag ? "#2f6db3" : "#cbd5e1"

    function fileName(fileUrl) {
        const value = fileUrl.toString()
        const separator = value.lastIndexOf("/")
        return separator >= 0 ? value.slice(separator + 1) : value
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 7

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: root.title
            color: "#1d2939"
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }

        Label {
            Layout.fillWidth: true
            text: root.hasSelection
                  ? root.fileName(root.selectedFile)
                  : root.acceptedSuffix.length > 0
                    ? "Drop one local " + root.acceptedSuffix + " file or click to browse"
                    : "Drop one local key file or click to browse"
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            color: root.hasSelection ? "#285a9a" : "#667085"
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            visible: root.hasSelection
            text: "Click to choose another file"
            color: "#667085"
            font.pixelSize: 12
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent

        onDropped: drop => {
            if (!drop.hasUrls)
                return

            const droppedUrls = drop.urls
            if (droppedUrls.length !== 1)
                return

            const fileUrl = droppedUrls[0]
            if (!fileUrl.toString().startsWith("file:") ||
                (root.acceptedSuffix.length > 0 &&
                 !root.fileName(fileUrl).toLowerCase().endsWith(root.acceptedSuffix)))
                return

            // Keep only the URL/path. File contents are never opened in QML.
            root.selectedFile = fileUrl

            // Never ask the source application to move a key file.
            drop.accept(Qt.CopyAction)
        }
    }

    TapHandler {
        onTapped: fileDialog.open()
    }

    Dialogs.FileDialog {
        id: fileDialog
        title: "Select " + root.title.toLowerCase() + " file"
        fileMode: Dialogs.FileDialog.OpenFile
        nameFilters: root.acceptedSuffix.length > 0
                     ? ["TrueSight files (*" + root.acceptedSuffix + ")"]
                     : ["All files (*)"]

        onAccepted: {
            if (selectedFile.toString().startsWith("file:") &&
                (root.acceptedSuffix.length === 0 ||
                 root.fileName(selectedFile).toLowerCase().endsWith(root.acceptedSuffix))) {
                root.selectedFile = selectedFile
            }
        }
    }
}
