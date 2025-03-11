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

    /******* Status Bar *********/

    StatusBar {
        id: statusBar
        nameofsession : root.sessionName
        nameOfport : root.portNumber
    }



    /************  Steering Wheel and Proximity Sener ************/

    Rectangle {
        id : leftRect
        width : parent.width / 4
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

        Rectangle {
            id: steeringWheelRect
            width: parent.width - 40
            height: parent.height / 3
            color: "#636363"
            border.color: "turquoise"
            border.width: 2
            radius: 20
            anchors {
                horizontalCenter : parent.horizontalCenter
                top : parent.top
                topMargin : 10

            }
            SteeringWheel {
                id: steeringWheel
                anchors.centerIn: parent
            }
        }


        Rectangle {
            id : proximityRect
            width : parent.width - 30
            height : parent.height / 2
            color : "#636363"
            border.color: "turquoise"
            border.width: 2
            radius: 20

            anchors {
                horizontalCenter : parent.horizontalCenter
                top : steeringWheelRect.bottom
                bottom : parent.bottom
                topMargin : 10
                bottomMargin : 10
            }

            Image {
                id : car
                source : "../Assets/AI_car_transparent.png"
                anchors.centerIn : parent
                height : parent.height - 30
                fillMode : Image.PreserveAspectFit
                smooth : true
            }
            WheelSpeed {
                id: fl
                wheelPos: "FL"
                scaleFactor : 1.1
                currentSpeed: udpClient.speedFL
                anchors {
                    top : car.top
                    left : proximityRect.left
                    topMargin : 70
                    leftMargin : 20
                }
            }
            WheelSpeed {
                id: fr
                wheelPos: "FR"
                scaleFactor : 1.1
                currentSpeed: udpClient.speedFR
                anchors {
                    top : car.top
                    right : proximityRect.right
                    topMargin : 70
                    rightMargin : 20
                }
            }

            WheelSpeed {
                id: bl
                wheelPos: "BL"
                scaleFactor : 1.1
                currentSpeed: udpClient.speedBL
                anchors {
                    bottom : car.bottom
                    left : proximityRect.left
                    bottomMargin : 70
                    leftMargin : 20
                }
            }

            WheelSpeed {
                id: br
                wheelPos: "BR"
                scaleFactor : 1.1
                currentSpeed: udpClient.speedBR
                anchors {
                    bottom : car.bottom
                    right : proximityRect.right
                    bottomMargin : 70
                    rightMargin : 20
                }
            }

            Connections {
                  target: udpClient
                  function onSpeedFRChanged() {
                      fr.currentSpeed = udpClient.speedFR;
                  }
                  function onSpeedFLChanged() {
                      fl.currentSpeed = udpClient.speedFL;
                  }
                  function onSpeedBRChanged() {
                      br.currentSpeed = udpClient.speedBR;
                  }
                  function onSpeedBLChanged() {
                      bl.currentSpeed = udpClient.speedBL;
                  }
            }
        }
    }

    /********************************************************/




    /******** Meters Screen *******/

    Rectangle {
        id: metersScreen
        width: speedometer.width + rpmMeter.width - 45
        height: 300
        color: "#09122C"
        border.color: "#D84040"
        border.width: 2

        anchors {
            top : statusBar.bottom
            topMargin: 8
            left : leftRect.right
            leftMargin : 70

        }

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


    /******** Battery , Accelator , Braker Pedal Readings *************/

    Rectangle {
        id : pedalTempRect
        width : parent.width / 5.1
        height : parent.height / 3.5

        color: "#09122C"
        border.color: "#D84040"
        border.width: 2
        radius : 20

        anchors {
            top : metersScreen.bottom
            topMargin : 65
            left : leftRect.right
            leftMargin : 30
        }

        AcceleratorPedal {
            id: acceleratorPedal
            pedalPosition: udpClient.accPedal
            anchors {
                bottom: parent.bottom
                left: parent.left
                margins: 10
            }
        }

        BrakePadel {
            id: brakePedal
            pedalPosition: udpClient.brakePedal
            anchors {
                bottom: parent.bottom
                left: acceleratorPedal.right
                margins: 10
            }
        }

        TemperatureIndicator {
            id: temperatureIndicator
            anchors {
                top: parent.top
                left: parent.left
                leftMargin: 10
                topMargin: 20
            }
        }

        BatteryLevelIndicator {
            id: batteryLevelIndicator
            scaleFactor: parent.height / 222.22
            anchors {
                right: parent.right
                top: parent.top
                rightMargin: 80
                topMargin: 75
            }
        }
    }


    /********************************************************/




    /*********** Right Rect (GPS) **********/

    Rectangle {
        id : rightRect
        width : parent.width / 4
        height : parent.height - 20 * root.scaleFactor
        color : "#09122C"
        radius : 30
        border.width : 2
        border.color : "#D84040"

        anchors {
            top : statusBar.bottom
            right: parent.right
            bottom : parent.bottom
            rightMargin : 12
            topMargin : 5
            bottomMargin : 12
        }

        Text {
            id : gpsText
            text : "GPS"
            color : "turquoise"
            font {
                pixelSize: 20
                family: "DS-Digital"
            }

            anchors {
                top : parent.top
                horizontalCenter : parent.horizontalCenter
                topMargin : 5
            }
        }

        GpsPlotter {
            id : gps
            anchors {
                horizontalCenter : parent.horizontalCenter
                top : parent.top
                bottom : parent.bottom
                topMargin : 30
                bottomMargin : 10

            }
        }

    }
    /********************************************************/


}

