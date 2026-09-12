import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string personName: ""
    property string previewText: ""
    property string timestamp: ""
    property int unreadCount: 0
    property bool selected: false

    implicitHeight: 68

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: root.selected ? "#eaf2ff" : "transparent"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
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
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: root.personName
                elide: Text.ElideRight
                font.weight: Font.DemiBold
                color: "#1d2939"
            }

            Label {
                Layout.fillWidth: true
                text: root.previewText
                elide: Text.ElideRight
                color: "#667085"
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            spacing: 5

            Label {
                text: root.timestamp
                color: "#667085"
                font.pixelSize: 12
            }

            Rectangle {
                visible: root.unreadCount > 0
                Layout.alignment: Qt.AlignRight
                implicitWidth: 18
                implicitHeight: 18
                radius: width / 2
                color: "#2f6db3"

                Label {
                    anchors.centerIn: parent
                    text: root.unreadCount
                    color: "white"
                    font.pixelSize: 11
                }
            }
        }
    }
}
