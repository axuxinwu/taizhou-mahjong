import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root
    property int tileCode: 0
    width: 40
    height: 56

    property int suit: (tileCode >> 4) & 0x0F
    property int value: tileCode & 0x0F

    property string faceText: {
        const fengNames = ["", "东","南","西","北"];
        const jianNames = ["", "中","发","白"];
        switch(suit) {
            case 0: return value + "\n筒";
            case 1: return value + "\n条";
            case 2: return value + "\n万";
            case 3: return fengNames[value] || "?";
            case 4: return jianNames[value] || "?";
            default: return "?";
        }
    }

    property color faceColor: {
        if (suit <= 2) return "ivory";
        if (suit === 3) return "#d6eaf8";
        if (suit === 4) return "#fef9e7";
        return "gray";
    }

    property color textColor: (suit === 4 && value === 2) ? "green" : "black";
    property int pointSize: (suit <= 2) ? 12 : 14;

    signal dragRelease

    Rectangle {
        anchors.fill: parent
        visible: tileCode != -1
        color: faceColor
        border.color: "#555"
        border.width: 1
        radius: 4

        Text {
            anchors.centerIn: parent
            text: faceText
            font.pixelSize: pointSize
            font.bold: suit >= 3
            color: textColor
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.1
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: tileCode == -1
        color: "#2c3e50"
        radius: 4
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.6; height: parent.height * 0.6
            color: "#5d6d7e"
            radius: 4
        }
    }

    MouseArea {
        anchors.fill: parent
        drag.target: parent
        drag.axis: Drag.YAxis
        drag.minimumY: -80
        drag.maximumY: 0
        onReleased: {
            if (parent.y < -40) {
                root.dragRelease();
                parent.y = 0;
            }
        }
    }
}