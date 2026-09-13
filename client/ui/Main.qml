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
    property var conversations: [
        {
            personName: "Maya Chen",
            previewText: "The design notes look good.",
            timestamp: "10:42",
            unreadCount: 2
        },
        {
            personName: "Alex Morgan",
            previewText: "I will send the updated draft shortly.",
            timestamp: "09:18",
            unreadCount: 0
        },
        {
            personName: "Jordan Patel",
            previewText: "Thanks, talk tomorrow.",
            timestamp: "Yesterday",
            unreadCount: 0
        },
        {
            personName: "Sam Rivera",
            previewText: "Could we review it this afternoon?",
            timestamp: "Mon",
            unreadCount: 1
        }
    ]
    property var messages: [
        {
            messageText: "Hi Maya, do you have a moment to review the latest design notes?",
            sentByMe: true,
            timestamp: "10:36"
        },
        {
            messageText: "Yes, I just finished reading them. The layout is much clearer now.",
            sentByMe: false,
            timestamp: "10:39"
        },
        {
            messageText: "Great. Is there anything you would change before we continue?",
            sentByMe: true,
            timestamp: "10:40"
        },
        {
            messageText: "The design notes look good. I added two small comments for you to consider.",
            sentByMe: false,
            timestamp: "10:42"
        }
    ]
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
            onContinueRequested: (username, privateKeyPath) => {
                // This demo stores the username and path; it does not open the file.
                //root.selectedUsername = username
                //root.selectedPrivateKeyPath = privateKeyPath
                //root.currentScreen = 1
                Auth.hello()
            }
        }

        MessengerScreen {
            conversations: root.conversations
            activeConversationName: root.activeConversationName
            activeConversationStatus: root.activeConversationStatus
            messages: root.messages
        }
    }
}
