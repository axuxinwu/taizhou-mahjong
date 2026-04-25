import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    visible: true
    width: 360
    height: 640
    title: "椒江麻将"

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: netComp
    }

    Component {
        id: netComp
        NetworkDialog {
            onHost: { gameController.hostGame(port); pageLoader.sourceComponent = tableComp }
            onJoin: { gameController.joinGame(ip, port); pageLoader.sourceComponent = tableComp }
        }
    }

    Component {
        id: tableComp
        TableView { anchors.fill: parent }
    }
}