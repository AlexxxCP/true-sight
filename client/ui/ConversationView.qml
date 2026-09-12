import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    // Each item may supply messageText, sentByMe, and timestamp.
    property string conversationName: ""
    property string conversationStatus: ""
    property var messages: []

    padding: 20

    background: Rectangle {
        color: "#ffffff"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        ChatHeader {
            Layout.fillWidth: true
            personName: root.conversationName
            status: root.conversationStatus
        }

        ListView {
            id: messageView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: root.messages

            delegate: MessageBubble {
                width: messageView.width
                messageText: modelData.messageText || ""
                sentByMe: modelData.sentByMe || false
                timestamp: modelData.timestamp || ""
            }

            Label {
                anchors.centerIn: parent
                visible: !root.conversationName.length
                text: "Select a conversation"
                color: "#667085"
            }
        }

        MessageComposer {
            Layout.fillWidth: true
        }
    }
}
