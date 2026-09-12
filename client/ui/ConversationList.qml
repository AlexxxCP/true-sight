import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    // Each item may supply personName, previewText, timestamp, and unreadCount.
    property var conversations: []

    padding: 16

    background: Rectangle {
        color: "#ffffff"
        border.color: "#e4e7ec"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Label {
            text: "Conversations"
            font.pixelSize: 17
            font.weight: Font.DemiBold
            color: "#1d2939"
        }

        ConversationFilterInput {
            Layout.fillWidth: true
        }

        ListView {
            id: conversationView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.conversations

            delegate: ConversationEntry {
                width: conversationView.width
                personName: modelData.personName || ""
                previewText: modelData.previewText || ""
                timestamp: modelData.timestamp || ""
                unreadCount: modelData.unreadCount || 0
            }

            Label {
                anchors.centerIn: parent
                visible: conversationView.count === 0
                text: "No conversations yet"
                color: "#667085"
            }
        }
    }
}
