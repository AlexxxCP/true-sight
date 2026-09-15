import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 1000
    height: 700
    minimumWidth: 800
    minimumHeight: 560
    visible: true
    title: "TrueSight"
    color: "#f7f8fa"
    palette.window: "#f7f8fa"
    palette.windowText: "#1d2939"
    palette.base: "#ffffff"
    palette.text: "#1d2939"
    palette.placeholderText: "#667085"

    // Temporary visual data. Replace these bindings with C++ Q_PROPERTY values.
    property string activeConversationName: "Maya Chen"
    property string activeConversationStatus: "Available"

    // UI-only navigation demo. Later, C++ should control this after it validates
    // the username and selected private-key file.
    property int currentScreen: 0
    property string selectedUsername
    property url selectedPrivateKeyPath

    header: AppHeader { }

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentScreen

        KeySelectionScreen {
            onContinueRequested: (username, authFile) => {
                authController.auth(username, authFile)
            }
        }

        Connections {
            target: authController

            function onAuthFinished(success) {
                if (success) {
                    root.currentScreen = 1
                }
            }

            function onAuthFailed(error) {
                console.log("Auth failed: ", error);
            }
        }

        Connections {
            target: messengerController

            function onMessageSent() {
                console.log("MESSAGE SENT")
            }

            function onMessageSentFailed(error) {
                console.error("MESSAGE SEND FAILED:", error)
            }
        }

        MessengerScreen {
            conversations: messengerController.conversations
            messages: messengerController.messages
            activeConversationName: messengerController.peer

            activeConversationStatus: root.activeConversationStatus
        }
    }
}
