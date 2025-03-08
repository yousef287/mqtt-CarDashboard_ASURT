import QtQuick

Rectangle {
    id : root

    width: 15
    height: 50
    color: "#E0E0E0"
    border.color: "black"
    border.width: 2
    property int maxSpeed: 200 // Max speed
    property int currentSpeed
    // Current speed (0-200)
    required property string wheelPos
    property real scaleFactor
    Rectangle {
        id: progressBar
        width: parent.width - 4
        height: parent.height * (parent.currentSpeed
                                 / parent.maxSpeed) // Adjust height based on speed
        color: root.getColorForSpeed(
                   root.currentSpeed) //parent.currentSpeed < 60 ? "green" : parent.currentSpeed < 100 ? "yellow" : "red"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 2
    }
    Text {
        id: pos
        text: root.wheelPos + ": "
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: speedText.top
        font.pixelSize: 14 * root.scaleFactor
        color: "turquoise"
    }

    Text {
        id: speedText
        text: parent.currentSpeed + " km/h"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        font.pixelSize: 16 * root.scaleFactor
        font.family: "DS-Digital"
        color: root.getColorForSpeed(root.currentSpeed)
    }



    // Function to interpolate color based on speed
    function getColorForSpeed(speed) {
        var r, g, b
        if (speed <= 100) {
            // Adjusted range for green to yellow
            // Interpolating from green (0, 255, 0) to yellow (255, 255, 0)
            r = Math.floor(255 * (speed / 100))
            // Red increases from 0 to 255
            g = 255
            // Green stays full
            b = 0
        } else if (speed <= 200) {
            // Adjusted range for yellow to red
            // Interpolating from yellow (255, 255, 0) to red (255, 0, 0)
            r = 255
            // Red stays full
            g = Math.floor(255 * (1 - (speed - 100) / 100))
            // Green decreases
            b = 0
            // Blue stays 0
        } else {
            // Above 120
            r = 200
            g = 0
            b = 0
        }
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(
                    1).toUpperCase()
    }
}
