import QtQuick
import QtQuick.Controls

ToolBar {
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
}
