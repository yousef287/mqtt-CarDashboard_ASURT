import QtQuick
import QtQuick.Controls
import "../StatusBar"
import "../.."

Rectangle {
    id: root

    // Add properties to receive data from WelcomeScreen
    property real scaleFactor: 1.0
    property string sessionName: ""
    property string portNumber: ""

    color: "#1A3438"
    anchors.fill: parent
    radius: 40
    border.color: "#A6F1E0"
    border.width: 5

    // Add Timer component for auto-navigation
    Timer {
        id: navigationTimer
        interval: 500  // 10 seconds
        running: true    // Start timer automatically when the page loads
        repeat: false    // Run only once
        onTriggered: {
            stackView.push("../InformationPage/Information.qml", {
                "sessionName": root.sessionName,
                "portNumber": root.portNumber
            })
        }
    }

    Text {
        id: waitingText
        text: "Loading..."
        font {
            family: "Amiri"
            bold: true
            pixelSize: 30 * root.scaleFactor
        }
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: busyIndicator.top
            bottomMargin: 10
        }
        color: "turquoise"
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: 70
        height: 60
    }

    StatusBar {
        id: statusBar
        nameofsession : root.sessionName
        nameOfport : root.portNumber

    }

    MyButton {
        id: backButton
        source: "../Assets/back-button.png"
        hoverText: "Back"
        smooth: true
        anchors {
            left: parent.left
            leftMargin: 12
            bottom: parent.bottom
            bottomMargin: 40
        }
        onClicked: {
            // Stop the timer
            navigationTimer.stop()
            // Tell the UDPClient to stop before popping back
            udpClient.stop()
            stackView.pop()
        }
    }

    Text {
        id: backText
        text: "Back"
        font {
            family: "DS-Digital"
            bold: true
            pixelSize: 18
        }
        color: "turquoise"
        anchors {
            horizontalCenter: backButton.horizontalCenter
            top: backButton.bottom
            topMargin: 5
        }
    }
}
