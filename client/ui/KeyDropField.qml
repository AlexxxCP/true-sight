import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property string title
    property url selectedFile

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
                  : "Drop one local key file here"
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            color: root.hasSelection ? "#285a9a" : "#667085"
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            visible: root.hasSelection
            text: "Path selected"
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

            // Keep only the URL/path. File contents are never opened in QML.
            root.selectedFile = droppedUrls[0]

            // Never ask the source application to move a key file.
            drop.accept(Qt.CopyAction)
        }
    }
}
