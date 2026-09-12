import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string messageText: ""
    property string timestamp: ""
    property bool sentByMe: false

    implicitHeight: bubble.height

    Rectangle {
        id: bubble
        anchors.right: root.sentByMe ? parent.right : undefined
        anchors.left: root.sentByMe ? undefined : parent.left
        width: Math.min(root.width * 0.72,
                        Math.max(110, Math.min(messageLabel.implicitWidth + 28, 500)))
        height: messageColumn.implicitHeight + 20
        radius: 12
        color: root.sentByMe ? "#2f6db3" : "#eef1f5"

        Column {
            id: messageColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 10
            spacing: 4

            Label {
                id: messageLabel
                width: parent.width
                text: root.messageText
                wrapMode: Text.Wrap
                color: root.sentByMe ? "#ffffff" : "#1d2939"
            }

            Label {
                visible: root.timestamp.length > 0
                text: root.timestamp
                color: root.sentByMe ? "#dbeafe" : "#667085"
                font.pixelSize: 11
            }
        }
    }
}
