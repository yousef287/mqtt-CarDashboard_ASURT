import QtQuick 2.15
import QtQuick.Controls
import "../StatusBar"

Rectangle {
    id: root

    property real scaleFactor : 1.0
    property string sessionName: ""
    property string portNumber: ""

    color: "#1A3438"
    anchors.fill: parent
    radius: 40
    border.color: "#A6F1E0"
    border.width: 5

    StatusBar {
        id: statusBar
        nameofsession : root.sessionName
        nameOfport : root.portNumber
    }


    Rectangle {
        id : leftRect
        width : parent.width / 3
        height : parent.height - 20 * root.scaleFactor
        color : "#09122C"
        radius : 30
        border.width : 2
        border.color : "#D84040"

        anchors {
            top : statusBar.bottom
            left: parent.left
            bottom : parent.bottom
            leftMargin : 12
            topMargin : 5
            bottomMargin : 12
        }

    }

    /*
    // Meters section for speedometer and RPM meter
    Rectangle {
        id: metersScreen
        width: speedometer.width + rpmMeter.width - 45
        height: 300
        color: "#09122C"
        border.color: "black"
        border.width: 2
        anchors.top: statusBar.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 20

        Speedometer {
            id: speedometer
            speed: udpClient.speed
            anchors {
                left: parent.left
                leftMargin: -20
                verticalCenter: parent.verticalCenter
                top: parent.top
            }
        }

        RpmMeter {
            id: rpmMeter
            rpm: udpClient.rpm
            anchors {
                left: speedometer.right
                right: parent.right
                rightMargin: 30
                verticalCenter: parent.verticalCenter
            }
        }
    }

    // Steering wheel section
    Rectangle {
        id: steeringWheelRect
        width: 200
        height: 250
        color: "#09122C"
        border.color: "black"
        border.width: 2
        radius: 20
        anchors {
            top: statusBar.bottom
            left: parent.left
            margins: 5
        }
        SteeringWheel {
            id: steeringWheel
            anchors.centerIn: parent
        }
    }
    */

}

