import QtQuick 2.15

Item {
    id: root
    property real temperature: 0.0

    Image {
        id: temperatureIndicatorImage
        source: "../Assets/thermometer.png"
        height: 70
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Text {
        id: temperatureText
        text: root.temperature.toFixed(1) + " °C"
        color: "turquoise"
        font {
            bold: true
            family: "DS-Digital"
            pixelSize: 22
        }
        anchors {
            bottom: temperatureIndicatorImage.bottom
            horizontalCenter: temperatureIndicatorImage.horizontalCenter
            bottomMargin: -25
        }
    }

    Connections {
        target: udpClient
        onTemperatureChanged: {
            temperature = udpClient.temperature;
        }
    }
}
