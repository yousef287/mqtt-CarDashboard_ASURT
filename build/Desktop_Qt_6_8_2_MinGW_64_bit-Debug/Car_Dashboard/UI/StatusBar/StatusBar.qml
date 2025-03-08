import QtQuick

Rectangle {
    id: root
    property string nameofsession : " "
    property string nameOfport : " "

    width: 1000
    height: 35

    color: "#09122C"
    radius: 9
    border.color: "#A6F1E0"
    border.width: 5
    anchors {
        top : parent.top
        horizontalCenter : parent.horizontalCenter
    }



    Text {
        id: timeText
        text: Qt.formatDateTime(new Date(), "hh:mm A") // Formats time dynamically
        color: "white"
        font.pixelSize: 25
        font.family : "DS-Digital"
        font.bold : true
        anchors.centerIn : parent
    }

    Text {
        id : sessionNameText
        text : "Session Name : " + root.nameofsession
        color : "white"
        font {
            family : "DS-Digital"
            pixelSize : 18
        }
        anchors {
            verticalCenter : parent.verticalCenter
            left : parent.left
            leftMargin : 10
        }
    }

    Text {
        id : portNumberText
        text : "Port Number : " + root.nameOfport
        color : "white"
        font {
            family : "DS-Digital"
            pixelSize : 18
        }
        anchors {
            verticalCenter : parent.verticalCenter
            right : parent.right
            rightMargin : 10
        }
    }

    Timer {
        interval: 1000 // Updates every second
        running: true
        repeat: true
        onTriggered: timeText.text = Qt.formatDateTime(new Date(), "hh:mm A")
    }




}
