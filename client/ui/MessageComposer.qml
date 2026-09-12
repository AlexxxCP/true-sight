import QtQuick

Rectangle {
    id: root

    property alias draftText: draftInput.text
    property alias text: draftInput.text
    readonly property bool hasDraft: draftInput.text.trim().length > 0
    signal sendRequested()

    implicitHeight: Math.min(152, Math.max(44, draftInput.contentHeight + 16))
    radius: 22
    color: "#f1f3f5"
    border.color: "#e0e4e8"

    Flickable {
        id: inputViewport

        anchors.left: parent.left
        anchors.right: sendButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 14
        anchors.rightMargin: 8
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        clip: true
        contentWidth: width
        contentHeight: draftInput.y + draftInput.height
        boundsBehavior: Flickable.StopAtBounds

        TextEdit {
            id: draftInput

            width: inputViewport.width
            height: Math.max(20, contentHeight)
            y: Math.max(0, (inputViewport.height - height) / 2)
            color: "#1d2939"
            wrapMode: TextEdit.Wrap
            selectByMouse: true
        }

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            visible: !draftInput.text.length
            text: "Write a message"
            color: "#667085"
        }
    }

    Rectangle {
        id: sendButton

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        width: 70
        height: 34
        radius: height / 2
        color: root.hasDraft ? "#2f6db3" : "#9db9d7"

        Text {
            anchors.centerIn: parent
            text: "Send"
            color: "#ffffff"
            font.weight: Font.DemiBold
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.hasDraft
            onClicked: root.sendRequested()
        }
    }
}
