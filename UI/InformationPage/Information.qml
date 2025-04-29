import QtQuick 2.15
import QtQuick.Shapes
import "../StatusBar"

Rectangle {
    id: root

    property real scaleFactor : 1.0
    property string sessionName: ""
    property string portNumber: ""
    property real maxLateralG: 3.5  // Maximum lateral G-force (cornering)
    property real maxLongitudinalG: 2.0  // Maximum longitudinal G-force (acceleration)
    property real maxBrakingG: 3.5  // Maximum braking G-force
    property real xDiagram: udpClient ? ((udpClient.lateralG / maxLateralG) * (ggImage.width / 2 - 20)) : 0
    property real yDiagram: udpClient ? ((udpClient.longitudinalG / maxLongitudinalG) * (ggImage.height / 2 - 20)) : 0

    color: "#1A3438"
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
                currentSpeed: udpClient ? udpClient.speedFL : 0
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
                currentSpeed: udpClient ? udpClient.speedFR : 0
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
                currentSpeed: udpClient ? udpClient.speedBL : 0
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
                currentSpeed: udpClient ? udpClient.speedBR : 0
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
        height: 335
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
            speed: udpClient ? udpClient.speed : 0
            anchors {
                left: parent.left
                leftMargin: -20
                verticalCenter: parent.verticalCenter
                top: parent.top
            }
        }

        RpmMeter {
            id: rpmMeter
            rpm: udpClient ? udpClient.rpm : 0
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
            pedalPosition: udpClient ? udpClient.accPedal : 0
            anchors {
                bottom: parent.bottom
                left: parent.left
                margins: 10
            }
        }

        BrakePadel {
            id: brakePedal
            pedalPosition: udpClient ? udpClient.brakePedal : 0
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


    /****** IMU *****/

    Rectangle {
        id : bottomRect

        width : parent.width / 3
        height : parent.height / 2.8
        color: "#09122C"
        border.color: "#D84040"
        border.width: 2
        radius : 30

        anchors {
            top : metersScreen.bottom
            left : pedalTempRect.right
            right : rightRect.left
            bottom : parent.bottom
            margins : 10
        }


        Image {
            id : ggImage
            source : "../Assets/GG_Diagram.png"
            width : 294
            height : 294
            anchors.centerIn : parent
            anchors.margins : 8
            rotation : -90
            fillMode : Image.PreserveAspectFit
            smooth : true

            // Calculate center point dynamically
            property real centerPoint: (ggImage.width - anchors.margins * 2) / 2

            ListModel {
                id: pathPoints
                Component.onCompleted: {
                    append({"x": ggImage.centerPoint, "y": ggImage.centerPoint}) // Dynamic center point
                }
            }

            Shape {
                id: pathShape
                anchors.fill: parent
                smooth: true
                antialiasing: true
                ShapePath {
                    id: movementPath
                    strokeWidth: 2
                    strokeColor: "#A6F1E0"
                    strokeStyle: ShapePath.SolidLine
                    fillColor: "transparent"
                    startX: pathPoints.get(0).x
                    startY: pathPoints.get(0).y

                    PathPolyline {
                        path: pathPoints
                    }
                }
            }

            Image {
                id: pointImage
                source: "../Assets/point.png"
                // Use dynamic center point for positioning
                x: root.yDiagram + ggImage.centerPoint
                y: root.xDiagram + ggImage.centerPoint
                width: 20
                height: 20
                fillMode: Image.PreserveAspectFit
                smooth: true
                z: 2

                // Add smooth animations for x and y movements
                Behavior on x {
                    SmoothedAnimation {
                        easing.type: Easing.InOutQuad
                        velocity: 200
                    }
                }
                Behavior on y {
                    SmoothedAnimation {
                        easing.type: Easing.InOutQuad
                        velocity: 200
                    }
                }

                Connections {
                    target: root
                    function onXDiagramChanged() {
                        if (pathPoints.count > 100) {
                            pathPoints.remove(0)
                        }
                        pathPoints.append({
                            "x": root.yDiagram + ggImage.centerPoint,
                            "y": root.xDiagram + ggImage.centerPoint
                        })
                    }
                    function onYDiagramChanged() {
                        if (pathPoints.count > 100) {
                            pathPoints.remove(0)
                        }
                        pathPoints.append({
                            "x": root.yDiagram + ggImage.centerPoint,
                            "y": root.xDiagram + ggImage.centerPoint
                        })
                    }
                }
            }
        }

        // Add acceleration text displays
        Row {
            anchors {
                top: ggImage.bottom
                horizontalCenter: ggImage.horizontalCenter
                topMargin: 15
            }
            spacing: 20

            Text {
                text: "Ax: " + root.xDiagram.toFixed(2) + " G"
                color: "white"
                font {
                    family: "Arial"
                    pixelSize: 16
                    bold: true
                }
            }

            Text {
                text: "Ay: " + root.yDiagram.toFixed(2) + " G"
                color: "white"
                font {
                    family: "Arial"
                    pixelSize: 16
                    bold: true
                }
            }
        }




        /*

        Rectangle {
            id : imuRect
            color : "#636363"
            border.width : 2
            border.color : "turquoise"
            anchors{
                fill : parent
                centerIn : parent
                margins : 8
            }

            radius : 20

            Connections {
                target: udpClient

                function onYawAngleChanged() {
                    yawGauge.eulerAngle = udpClient.yawAngle
                    yawGauge.rePaint()
                }
                function onPitchAngleChanged() {
                    pitchgauge.eulerAngle = udpClient.pitchAngle
                    pitchgauge.rePaint()
                }
                function onRollAngleChanged() {
                    rollgauge.eulerAngle = udpClient.rollAngle
                    rollgauge.rePaint()
                }
            }

            EulerGauges {
                id : yawGauge
                textVal: "Yaw (X)"
                scaleFactor : parent.height / 205
                anchors {
                   left : parent.left
                   top : parent.top
                   topMargin : 8
                   leftMargin : imuRect.height / 4.8
                }

            }

            EulerGauges {
                id : pitchgauge
                textVal: "Pitch(Y)"
                scaleFactor : parent.height / 205
                anchors {
                   right : parent.right
                   top : parent.top
                   topMargin : 8
                   rightMargin : parent.height / 4.5
                }

            }

            EulerGauges {
                id : rollgauge
                textVal: "Roll(Z)"
                scaleFactor : parent.height / 205
                anchors {
                   left : parent.left
                   top : yawGauge.bottom
                   topMargin : parent.height / 2.8
                   leftMargin : parent.height / 4.8
                }

            }

            EulerVisual {
                anchors {
                    right : parent.right
                    top : parent.top
                    topMargin : 200
                }
            }
        }
        */



    }




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

