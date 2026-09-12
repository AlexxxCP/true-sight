import QtQuick

Rectangle {
    id: root

    implicitHeight: 36
    color: "#ffffff"
    radius: 6
    border.color: "#d0d5dd"

    Flickable {
        id: searchViewport
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        clip: true
        contentWidth: width
        contentHeight: height

        TextEdit {
            id: searchInput
            width: searchViewport.width
            height: searchViewport.height
            color: "#ffffff"
            selectionColor: "#b9d6f7"
            selectedTextColor: "#ffffff"
            wrapMode: TextEdit.NoWrap
            selectByMouse: true
        }

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            visible: !searchInput.text.length
            text: "Search conversations"
            color: "#ffffff"
        }
    }
}
