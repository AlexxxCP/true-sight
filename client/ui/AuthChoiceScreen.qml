import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts

Pane {
    id: root

    signal registerRequested()
    signal loginRequested()
    property string errorMessage: ""

    background: Rectangle { color: "#f7f8fa" }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12

        RowLayout {
            spacing: 16

            Basic.Button {
                id: registerButton
                implicitWidth: 160
                implicitHeight: 46
                text: "Register"

                contentItem: Text {
                    text: registerButton.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.weight: Font.DemiBold
                }

                background: Rectangle {
                    radius: 8
                    color: registerButton.down ? "#1f4f86"
                         : registerButton.hovered ? "#285f9e" : "#2f6db3"
                }

                onClicked: root.registerRequested()
            }

            Basic.Button {
                id: loginButton
                implicitWidth: 160
                implicitHeight: 46
                text: "Login"

                contentItem: Text {
                    text: loginButton.text
                    color: "#285a9a"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.weight: Font.DemiBold
                }

                background: Rectangle {
                    radius: 8
                    color: loginButton.hovered ? "#eff6ff" : "#ffffff"
                    border.color: "#b9d6f7"
                }

                onClicked: root.loginRequested()
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.errorMessage.length > 0
            text: root.errorMessage
            wrapMode: Text.WordWrap
            color: "#b42318"
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
