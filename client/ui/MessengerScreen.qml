import QtQuick
import QtQuick.Controls

SplitView {
    id: root

    property var conversations: []
    property var messages: []
    property string activeConversationName: ""
    property string activeConversationStatus: ""
    property string messageLoadError: ""

    handle: Rectangle {
        implicitWidth: 6
        color: "transparent"

        Rectangle {
            anchors.centerIn: parent
            width: 1
            height: parent.height
            color: "#e4e7ec"
        }
    }

    ConversationList {
        SplitView.preferredWidth: 310
        SplitView.minimumWidth: 240
        conversations: root.conversations
    }

    ConversationView {
        SplitView.fillWidth: true
        conversationName: root.activeConversationName
        conversationStatus: root.activeConversationStatus
        messages: root.messages
        messageLoadError: root.messageLoadError
    }
}
