import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic as Basic

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

        Basic.Button {
            id: addConversationButton
            Layout.fillWidth: true
            implicitHeight: 38
            text: "Add conversation +"

            contentItem: Text {
                text: addConversationButton.text
                color: "#285a9a"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.DemiBold
            }

            background: Rectangle {
                radius: 7
                color: addConversationButton.hovered ? "#eff6ff" : "#ffffff"
                border.color: "#b9d6f7"
            }

            onClicked: {
                shareFileField.selectedFile = ""
                shareError.text = ""
                addConversationDialog.open()
            }
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
                personName: modelData.peer || ""
                previewText: ""
                timestamp: modelData.last_message_at || null
                unreadCount: 0

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        messengerController.openConversation(modelData.peer)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: conversationView.count === 0
                text: "No conversations yet"
                color: "#667085"
            }
        }
    }

    Dialog {
        id: addConversationDialog
        parent: Overlay.overlay
        modal: true
        title: "Add conversation"
        width: Math.min(420, parent.width - 40)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        standardButtons: Dialog.Cancel

        ColumnLayout {
            width: parent.width
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: "Drop the other person's public key share file."
                wrapMode: Text.WordWrap
                color: "#667085"
            }

            KeyDropField {
                id: shareFileField
                Layout.fillWidth: true
                title: "Public key"
                acceptedSuffix: ".share"
            }

            Label {
                id: shareError
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.WordWrap
                color: "#b42318"
            }

            Basic.Button {
                id: addButton
                Layout.fillWidth: true
                implicitHeight: 40
                text: "Add"
                enabled: shareFileField.hasSelection

                contentItem: Text {
                    text: addButton.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.weight: Font.DemiBold
                }

                background: Rectangle {
                    radius: 7
                    color: addButton.enabled ? "#2f6db3" : "#98a2b3"
                }

                onClicked: {
                    const error = messengerController.addConversationFromShare(
                        shareFileField.selectedFile)
                    if (error.length > 0) {
                        shareError.text = error
                    } else {
                        addConversationDialog.close()
                    }
                }
            }
        }
    }
}
