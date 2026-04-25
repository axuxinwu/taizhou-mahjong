import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    color: "#2d5a27"

    signal host(int port)
    signal join(string ip, int port)

    Column {
        anchors.centerIn: parent
        spacing: 20
        Text { text: "椒江麻将"; font.pixelSize: 32; color: "gold" }
        Button {
            text: "创建房间"
            onClicked: host(12345)
        }
        TextField { id: ipField; placeholderText: "服务器IP" }
        Button {
            text: "加入房间"
            onClicked: join(ipField.text, 12345)
        }
    }
}