import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    // Each item may supply messageText, sentByMe, and timestamp.
    property string conversationName: ""
    property string conversationStatus: ""
    property var messages: []
    property string messageLoadError: ""

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

        Label {
            Layout.fillWidth: true
            visible: root.messageLoadError.length > 0
            text: root.messageLoadError
            color: "#b42318"
            wrapMode: Text.WordWrap
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
                messageText: modelData.message_text || ""
                sentByMe: modelData.receiver_iid === root.conversationName
                timestamp: modelData.created_at || null
            }

            Label {
                anchors.centerIn: parent
                visible: !root.conversationName.length
                text: "Select a conversation"
                color: "#667085"
            }
        }

        MessageComposer {
            id: messageComposer
            Layout.fillWidth: true
            enabled: root.conversationName.length > 0

            onSendRequested: {
                const outgoingMessage = draftText

                if (outgoingMessage.trim().length === 0)
                    return

                messengerController.sendMessage(outgoingMessage)
                draftText = ""
            }
        }
  }
}
