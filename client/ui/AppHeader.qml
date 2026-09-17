import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic

ToolBar {
    id: root
    property bool logoutVisible: false
    signal logoutRequested()

    height: 56

    background: Rectangle {
        color: "#ffffff"
        border.color: "#e4e7ec"
        border.width: 1
    }

    Label {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 20
        text: "TrueSight"
        font.pixelSize: 20
        font.weight: Font.DemiBold
        color: "#1d2939"
    }

    Basic.Button {
        id: logoutButton
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        visible: root.logoutVisible
        text: "Logout"

        contentItem: Text {
            text: logoutButton.text
            color: "#285a9a"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 7
            color: logoutButton.hovered ? "#eff6ff" : "#ffffff"
            border.color: "#b9d6f7"
        }
        onClicked: root.logoutRequested()
    }
}
