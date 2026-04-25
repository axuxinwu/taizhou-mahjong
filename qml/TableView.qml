import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    color: "#2d5a27"

    // 弃牌区
    ListView {
        id: discardArea
        anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 100 }
        height: 100
        orientation: ListView.Horizontal
        model: gameController.discardedTiles
        delegate: Tile {
            tileCode: modelData
            scale: 0.8
            MouseArea { anchors.fill: parent; enabled: false } // 不可操作
        }
    }

    // 手牌区
    ListView {
        id: handList
        anchors { bottom: toolBar.top; left: parent.left; right: parent.right }
        height: 80
        orientation: ListView.Horizontal
        model: gameController.playerHand
        delegate: Tile {
            tileCode: modelData
            width: 40; height: 56
            onDragRelease: {
                if (y < -40) {
                    gameController.discardTile(modelData);
                    y = 0;
                }
            }
        }
    }

    // 按钮栏
    Row {
        id: toolBar
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
        spacing: 10
        ActionButton { text: "摸牌"; onClicked: gameController.drawTile() }
        ActionButton { text: "吃";   onClicked: gameController.chi() }
        ActionButton { text: "碰";   onClicked: gameController.peng() }
        ActionButton { text: "杠";   onClicked: gameController.gang() }
        ActionButton { text: "胡";   onClicked: gameController.hu() }
        ActionButton { text: "过";   onClicked: gameController.pass() }
    }
}