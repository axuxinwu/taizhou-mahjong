import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    id: root
    property color normalColor: "#f0d9b5"
    property color pressedColor: "#d18b47"
    property color textColor: "#2d5a27"
    property int fontSize: 16

    contentItem: Text {
        text: root.text
        font.pixelSize: fontSize
        font.bold: true
        color: textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: root.pressed ? pressedColor : normalColor
        border.color: "#8b5a2b"
        border.width: 2
        radius: 8
    }
}