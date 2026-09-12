import QtQuick

Rectangle {
    id: root

    implicitHeight: 38
    color: "#ffffff"
    radius: 7
    border.width: 1
    border.color: "#cbd5e1"

    TextInput {
        id: input

        anchors.fill: parent
        anchors.leftMargin: 11
        anchors.rightMargin: 11
        color: "#1d2939"
        selectionColor: "#b9d6f7"
        selectedTextColor: "#1d2939"
        verticalAlignment: TextInput.AlignVCenter
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 11
        anchors.verticalCenter: parent.verticalCenter
        visible: input.text.length === 0
        text: "Filter conversations"
        color: "#667085"
    }
}
