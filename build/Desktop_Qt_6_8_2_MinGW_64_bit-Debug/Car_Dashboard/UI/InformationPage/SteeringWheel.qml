import QtQuick 2.15

Item {
    id: root
    property double encoderAngle: udpClient ? udpClient.encoderAngle : 0
    property real scaleFactor: 1.0


    Text {
        anchors {
            bottom: steeringwheel.top
            bottomMargin: -10
            horizontalCenter: parent.horizontalCenter
        }
        text: "Steering Angle: " + root.encoderAngle + "°"
        font {
            pixelSize: 20 * root.scaleFactor
            family: "DS-Digital"
        }
        color: "turquoise"
    }

    Image {
        id: steeringwheel
        source: "../Assets/Steering_wheel.png"
        rotation: root.encoderAngle
        anchors.centerIn: parent
        width: 220 * root.scaleFactor
        height: 180 * root.scaleFactor
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Connections {
        target: udpClient
        onEncoderAngleChanged: {
            encoderAngle = udpClient.encoderAngle;
        }
    }
}
