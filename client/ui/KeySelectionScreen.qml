import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts

Pane {
    id: root

    signal continueRequested(string username, url privateKeyPath)

    padding: 40

    background: Rectangle {
        color: "#f7f8fa"
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(520, parent.width - 80)
        spacing: 18

        Label {
            Layout.fillWidth: true
            text: "Sign in"
            horizontalAlignment: Text.AlignHCenter
            color: "#1d2939"
            font.pixelSize: 26
            font.weight: Font.DemiBold
        }

        Label {
            Layout.fillWidth: true
            text: "Enter your username and select your private key. "
                  + "This screen stores only the key path."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: "#667085"
        }

        Basic.TextField {
            id: usernameField
            Layout.fillWidth: true
            implicitHeight: 44
            leftPadding: 14
            rightPadding: 14
            placeholderText: "Username"
            color: "#1d2939"
            placeholderTextColor: "#667085"
            selectByMouse: true

            background: Rectangle {
                radius: 10
                color: "#ffffff"
                border.width: 1
                border.color: usernameField.activeFocus ? "#2f6db3" : "#cbd5e1"
            }
        }

        KeyDropField {
            id: privateKeyField
            Layout.fillWidth: true
            title: "Private key"
        }

        Basic.Button {
            id: continueButton

            Layout.fillWidth: true
            Layout.topMargin: 4
            implicitHeight: 44
            text: "Continue"
            visible: usernameField.text.trim().length > 0
                     && privateKeyField.hasSelection

            contentItem: Text {
                text: continueButton.text
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.DemiBold
            }

            background: Rectangle {
                radius: 8
                color: continueButton.down
                       ? "#1f4f86"
                       : continueButton.hovered ? "#285f9e" : "#2f6db3"
            }

            onClicked: root.continueRequested(usernameField.text.trim(),
                                               privateKeyField.selectedFile)
        }
    }
}
