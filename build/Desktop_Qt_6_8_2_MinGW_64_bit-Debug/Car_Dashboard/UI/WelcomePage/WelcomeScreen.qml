import QtQuick 2.15
import QtQuick.Controls
import "../.."

Rectangle {
    id: root

    property real scaleFactor : 1.0
    color: "#1A3438"
    anchors.fill: parent
    radius: 40
    border.color: "#A6F1E0"
    border.width: 5

    /**** Layout for the Two Rectangles ***/

    Row {
        anchors.fill : parent
        anchors.margins : 40 * root.scaleFactor
        spacing : 10


        /**** Left Rectangle ****/

        Rectangle {
            id : leftRect
            width : parent.width / 3
            height : parent.height - 10 * root.scaleFactor
            color : "#09122C"
            radius : 30
            border.width : 1.5
            border.color : "#D84040"


            /**** Image of the Formula Student Car ****/


            Image {
                id : carImg
                source : "../Assets/CAR-215-ASURT.png"
                width : 350 * root.scaleFactor
                height : 300 * root.scaleFactor
                fillMode : Image.PreserveAspectFit
                smooth : true

                anchors {
                    top : formulalogo.top
                    horizontalCenter : parent.horizontalCenter
                    topMargin : 40
                }
            }


            /**** Formula logo ****/


            Image {
                id : formulalogo
                source : "../Assets/formulalogo.jpeg"
                width : 200
                height : 100
                fillMode : Image.PreserveAspectFit
                smooth : true
                anchors {
                    top : parent.top
                    left : parent.left
                    leftMargin : 10
                }
            }


            /**** Layout for the Components of this Rectangle ****/

            Column {
                spacing : 8

                anchors {
                    horizontalCenter : parent.horizontalCenter
                    top : carImg.bottom
                    topMargin : 10
                }

                /**** Field to Get Session Name ****/

                Text {
                    id : sessionInfo
                    text : "Please Enter Session details"
                    font {
                        family : "DS-Digital"
                        pixelSize : 18 * root.scaleFactor
                        bold : true
                    }

                    anchors.horizontalCenter : parent.horizontalCenter
                    color: "turquoise"
                }

                Text {
                    id : sessionName
                    text : "Enter Session Name : "
                    font {
                        pixelSize : 14 * root.scaleFactor
                        bold : true
                    }

                    anchors.left : parent.left
                    color : "white"
                }

                TextField {
                    id : sessionNameField
                    placeholderText: startButton.inValid_Name ? "Don't Leave This Empty!" : "Enter Session Name"
                    placeholderTextColor: startButton.inValid_Name ? "darkRed" : "turquoise"

                    width: 300 * root.scaleFactor
                    height: 30 * root.scaleFactor

                    font.pointSize : 12 * root.scaleFactor
                    verticalAlignment :  TextInput.AlignVCenter
                    anchors.horizontalCenter : parent.horizontalCenter
                    color : "white"

                    background : Rectangle {
                        color: "#636363"
                        radius: 50
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Name ? "darkRed" : "transparent"
                        border.width : 4
                    }

                }


                /**** Field to Get Port Number ****/


                Text {
                    text: "Choose Port:"
                    font {
                        bold: true
                        pixelSize: 14 * root.scaleFactor
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id : portField
                    placeholderText: startButton.inValid_Port ? "Don't Leave This Empty!" : "Enter Port (e.g. , 8080)"
                    placeholderTextColor: startButton.inValid_Port ? "darkRed" : "turquoise"

                    width: 300 * root.scaleFactor
                    height: 30 * root.scaleFactor

                    font.pointSize : 12 * root.scaleFactor
                    verticalAlignment :  TextInput.AlignVCenter
                    anchors.horizontalCenter : parent.horizontalCenter
                    color : "white"

                    background : Rectangle {
                        color: "#636363"
                        radius: 50
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Port ? "darkRed" : "transparent"
                        border.width : 4
                    }

                }
            }


            /**** Start Button ****/


            MyButton {
                id : startButton
                property bool inValid_Name : false
                property bool inValid_Port : false

                source : "../Assets/powerButton.png"
                hoverText : "Start Session"
                width : 50 * root.scaleFactor
                height : 50 * root.scaleFactor
                fillMode : Image.PreserveAspectFit
                smooth : true

                anchors {
                    horizontalCenter : parent.horizontalCenter
                    bottom : parent.bottom
                    bottomMargin : 50
                }

                onClicked : {
                    inValid_Name = (sessionNameField.text === "")
                    inValid_Port = (portField.text === "")

                    /* Navigate to the next page if there is an valid_session name and valid port number */

                    if(!inValid_Name && !inValid_Port) {
                        // Convert port text to integer
                        var portNumber = parseInt(portField.text.trim())

                        // Validate port number (valid ports are 1-65535)
                        if (isNaN(portNumber) || portNumber < 1 || portNumber > 65535) {
                            // Show error for invalid port
                            inValid_Port = true
                            return
                        }

                        // Start the UDP client with the entered port
                        var success = udpClient.start(portNumber)

                        if (success) {
                            console.log("UDP client started successfully on port: " + portNumber)
                            // Pass session name and port when navigating to waiting screen
                            stackView.push("WaitingScreen.qml", {
                                "sessionName": sessionNameField.text,
                                "portNumber": portField.text
                            })
                        } else {
                            console.error("Failed to start UDP client on port: " + portNumber)
                            // Optionally show an error message
                            inValid_Port = true // Mark as invalid to show error state
                        }
                    }
                }
            }

            Text {
                id : startText
                text : "Start"

                font {
                    family : "DS-Digital"
                    bold : true
                    pixelSize : 20 * root.scaleFactor
                }
                color: "turquoise"
                anchors {
                    horizontalCenter : parent.horizontalCenter
                    top : startButton.bottom
                    topMargin : 5
                }
            }
        }


        /**** Right Rectangle (Extra Information Rectangle)  ****/

        Rectangle {
            id : extraInfromationRect
            width : (2 * parent.width / 3) - 50
            height : parent.height - 10 * root.scaleFactor
            color : "#09122C"
            radius : 30
            border.width : 1.5
            border.color : "#D84040"


            /**** Road Image ****/

            Image {
                id : roadImg
                source : "../Assets/road2.png"
                width : 400 * root.scaleFactor
                height : 400 * root.scaleFactor
                fillMode : Image.PreserveAspectFit
                smooth : true
                anchors {
                    right : parent.right
                    verticalCenter : parent.verticalCenter
                }
            }

            /**** Car on The Road ****/

            Image {
                id : roadCarImg
                source : "../Assets/car3_white.png"
                width: 150 * root.scaleFactor
                height: 150 * root.scaleFactor
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.centerIn: roadImg
            }


            /**** Team Logo ****/

            Image  {
                id : racingTeamLogo
                source : "../Assets/racinglogo.png"
                width : 100 * root.scaleFactor
                height : 100 * root.scaleFactor
                fillMode : Image.PreserveAspectFit
                smooth : true

                anchors {
                    right : parent.right
                    top : parent.top

                    topMargin : 10
                    rightMargin : 10
                }
            }

            /**** Layout for the Fields ****/


            Column {
                id :col
                anchors {
                    left : parent.left
                    top : parent.top
                    margins : 20
                }
                spacing : 10

                /**** Field to Get Driver Name ****/

                Text {
                    id : extraText
                    text : "Extra Information"
                    font {
                        bold : true
                        pixelSize : 16 * root.scaleFactor
                        family : "Amiri"
                    }
                    color: "turquoise"
                }
                Text {
                    text: "Enter Driver Name:"
                    font {
                        bold: true
                        pixelSize: 13 * root.scaleFactor
                        family : "Amiri"
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }
                TextField {
                    id : driverNameField
                    placeholderText : "Enter Driver Name"
                    placeholderTextColor : "turquoise"
                    width : 300 * root.scaleFactor
                    height : 30 * root.scaleFactor

                    font.pointSize : 12 * root.scaleFactor
                    verticalAlignment : TextInput.AlignVCenter
                    anchors.left : parent.left
                    color: "white"
                    background: Rectangle {
                        color: "#636363"
                        radius: 100
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                         border.width: 4
                    }

                }


                /**** Area to Write the Session Goals ****/

                Text {
                    text: "Enter Session Goals:"
                    font {
                        bold: true
                        pixelSize: 13 * root.scaleFactor
                        family : "Amiri"
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }
                TextArea {
                    id: goalsField
                    width: 350 * root.scaleFactor
                    height: 250 * root.scaleFactor
                    wrapMode: Text.Wrap
                    placeholderText: "Write your goals here..."
                    placeholderTextColor: "turquoise"
                    font.pointSize: 13 * root.scaleFactor
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"
                    background: Rectangle {
                        color: "#636363"
                        radius: 10
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: 4
                    }
                }
            }

        }

    }
}
