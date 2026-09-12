import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root

    property string personName: ""
    property string status: ""

    visible: personName.length > 0
    spacing: 10

    Rectangle {
        Layout.preferredWidth: 38
        Layout.preferredHeight: 38
        radius: width / 2
        color: "#dbe7f7"

        Label {
            anchors.centerIn: parent
            text: root.personName.length ? root.personName.charAt(0).toUpperCase() : "?"
            color: "#285a9a"
            font.weight: Font.DemiBold
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 1

        Label {
            text: root.personName
            color: "#1d2939"
            font.pixelSize: 17
            font.weight: Font.DemiBold
        }

        Label {
            visible: root.status.length > 0
            text: root.status
            color: "#667085"
            font.pixelSize: 12
        }
    }
}
