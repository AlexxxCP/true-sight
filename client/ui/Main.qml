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
    property string messageLoadError: ""
    property string loginError: ""

    // UI navigation between registration, login, and messaging.
    property int currentScreen: 0
    property string registrationStartError: ""
    property string registrationError: ""
    property string downloadError: ""
    property bool registrationSubmitting: false
    property string registeredUsername: ""
    header: AppHeader {
        logoutVisible: root.currentScreen === 4
        onLogoutRequested: {
            root.registrationStartError = authController.logout()
            loginScreen.clearSelection()
            root.loginError = ""
            root.currentScreen = 0
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentScreen

        AuthChoiceScreen {
            errorMessage: root.registrationStartError
            onRegisterRequested: {
                root.registrationStartError = registrationController.beginRegistration()
                if (root.registrationStartError.length === 0) {
                    root.registrationError = ""
                    root.currentScreen = 2
                }
            }
            onLoginRequested: {
                root.loginError = ""
                root.currentScreen = 1
            }
        }

        KeySelectionScreen {
            id: loginScreen
            errorMessage: root.loginError
            onContinueRequested: (username, authFile) => {
                root.loginError = ""
                authController.auth(username, authFile)
            }
        }

        RegistrationScreen {
            errorMessage: root.registrationError
            submitting: root.registrationSubmitting
            onContinueRequested: username => {
                root.registrationError = ""
                root.registrationSubmitting = true
                registrationController.registerUsername(username)
            }
        }

        RegistrationDownloadsScreen {
            username: root.registeredUsername
            shareSaved: registrationController.shareSaved
            privateKeySaved: registrationController.privateKeySaved
            errorMessage: root.downloadError

            onSaveShareRequested: destination => {
                root.downloadError = registrationController.saveShare(destination)
            }
            onSavePrivateKeyRequested: destination => {
                root.downloadError = registrationController.savePrivateKey(destination)
            }
            onContinueRequested: {
                registrationController.finishRegistration()
                root.currentScreen = 1
            }
        }

        MessengerScreen {
            conversations: messengerController.conversations
            messages: messengerController.messages
            activeConversationName: messengerController.peer

            activeConversationStatus: root.activeConversationStatus
            messageLoadError: root.messageLoadError
        }
    }

    Connections {
        target: registrationController

        function onRegistrationSucceeded(username) {
            root.registrationSubmitting = false
            root.registeredUsername = username
            root.downloadError = ""
            root.currentScreen = 3
        }

        function onRegistrationFailed(error) {
            root.registrationSubmitting = false
            root.registrationError = error
        }
    }

    Connections {
        target: authController

        function onAuthFinished(success) {
            if (success) {
                root.currentScreen = 4
            }
        }

        function onAuthFailed(error) {
            root.loginError = error
            root.currentScreen = 1
        }
    }

    Connections {
        target: messengerController

        function onPeerChanged() {
            root.messageLoadError = ""
        }

        function onMessagesChanged() {
            root.messageLoadError = ""
        }

        function onMessagesLoadFailed(error) {
            root.messageLoadError = error
        }

        function onMessageSent() {
            console.log("MESSAGE SENT")
        }

        function onMessageSentFailed(error) {
            console.error("MESSAGE SEND FAILED:", error)
        }
    }

}
