# Real-Time Vehicle Telemetry Dashboard

A robust, real-time vehicle telemetry dashboard built using Qt/QML and UDP. This project receives and processes data from a vehicle (or simulation) including speed, RPM, GPS coordinates, wheel speeds, and other parameters, and displays them dynamically on an interactive UI.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Architecture](#architecture)
- [Technologies Used](#technologies-used)
- [Installation and Setup](#installation-and-setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [Screenshots and Demonstrations](#screenshots-and-demonstrations)
- [Testing](#testing)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [Contact Information](#contact-information)

## Introduction
The Real-Time Vehicle Telemetry Dashboard is a desktop application designed to monitor and display various vehicle parameters in real time. Using UDP for data transmission and Qt/QML for the user interface, the application provides a modern, responsive, and visually engaging experience. The project supports:

- Live transmission of vehicle telemetry data.
- Display of real-time metrics such as speed, RPM, GPS coordinates, temperature, battery level, and individual wheel speeds.
- A customizable dashboard that adapts to screen size and device resolution.
- Session management and data validation for secure and reliable operation.

## Features

### Real-Time Data Processing
- Receives live data via UDP and updates the dashboard in real time.

### Comprehensive Telemetry
Monitors various vehicle metrics including:
- **Vehicle speed and RPM**
- **Accelerator and brake pedal positions**
- **Steering encoder angle**
- **Temperature and battery level**
- **IMU Parameters**
- **GPS coordinates (longitude and latitude) with route mapping**
- **Four individual wheel speeds (FL, FR, BL, BR)**

### Interactive Map
- Uses QtLocation and QtPositioning to display the vehicle’s current location and route path.

### Responsive UI
- Dynamic layouts that scale with window size and device resolution.

### Data Validation and Error Handling
- Ensures proper input before starting sessions and provides meaningful error messages.

### Session Management
- Supports session creation, configuration, and transition to a waiting screen.

## Architecture

### UDP Data Handling
- **UdpClient:** Receives and parses incoming UDP datagrams, updating QML properties with telemetry data.
- **UdpSender:** (On the server side) Sends formatted UDP datagrams containing telemetry data.

### User Interface (UI)
Built with Qt/QML and divided into several modules:
- **Main Dashboard:** Displays overall telemetry.
- **Map Module:** Uses QtLocation to show the GPS route and location.
- **Wheel Speed Components:** Custom components (e.g., WheelSpeedCar) display individual wheel speeds.
- **Session Management:** Screens for entering session details and starting sessions.

### Data Validation
- Integrated logic to ensure input fields are correctly filled and formatted.

## Technologies Used
- **Qt/QML:** For a cross-platform, responsive user interface.
- **UDP Networking:** Using QUdpSocket for real-time data transmission.
- **QtLocation & QtPositioning:** For map display and geolocation.
- **C++:** Backend logic and UDP data processing.
- **Git:** Version control for source code management.

## Installation and Setup

### Prerequisites
- **Qt 6+** (with QtLocation and QtPositioning modules)
- **C++ Compiler:** Compatible with your Qt installation.
- **Git:** For cloning the repository.

### Cloning the Repository
```bash
git clone https://github.com/Mohamedkhaled687/CarDashboard_ASURT.git
```



## Configuration

### UDP Settings
- **UDP Client:** Configure the listening port in the UdpClient component.
- **UDP Sender:** Set the destination IP address and port in the UDP Sender screen.

### Map Configuration
- **Map Plugin:** This project uses the OpenStreetMap (OSM) plugin. Ensure your network connection is available to load map tiles.
- **GPS Mapping:** Adjust mapping boundaries in the `gps.qml` file if necessary.

### Telemetry Data Format
UDP datagrams should be formatted as follows:
```
speed,rpm,accPedal,brakePedal,encoderAngle,temperature,batteryLevel,gpsLongitude,gpsLatitude,speedFL,speedFR,speedBL,speedBR
```

## Usage

### Running the Dashboard
- Start the application.
- Enter the destination IP and port on the UDP Sender screen.
- Use the random slider to simulate telemetry data.
- Click **"Send Data"** to transmit the data over UDP.
- The dashboard updates in real time as new data is received.

### Session Management
- **Session Creation:** Enter session details such as session name and port.
- **Waiting Screen:** After validation, the application transitions to a waiting screen.

### Map Interaction
- Use the zoom in/out buttons to adjust the map’s zoom level.
- The map automatically centers on the latest GPS coordinate received.
- A blue line represents the vehicle’s route on the map.

## Screenshots and Demonstrations
- **Session Setup:**
    ![Image](https://github.com/user-attachments/assets/57af4040-6692-4ea8-bf3c-17cfe9a2edbb)
- **Dashboard Overview:**
    ![Image](https://github.com/user-attachments/assets/d0cbf5e2-0fa1-41af-9597-0eb4d85698ca)
    

## Testing

### Unit Testing
- Unit tests are included for critical components.
- Run tests using Qt Creator’s test framework.

### Manual Testing
- Verify UDP connectivity using a UDP packet generator.
- Test UI functionality by simulating random telemetry data via the provided slider.

## Project Structure
```
CarDashboard/
├── README.md
├── main.cpp
├── CMAKELists.txt
├── src/
│   ├── udpclient.h
│   ├── udpclient.cpp
├── qml/
│   ├── main.qml
│   ├── gps.qml
│   ├── wheelsBox.qml
│   ├── WaitingScreen.qml
│   └── ... (other QML files)
└── assets/
    ├── marker.png
    ├── AI_car_transparent.png
    ├── CAR-215-ASURT.png
    └── ... (other asset files)
```

## Contributing
Contributions are welcome! To contribute:
1. Fork the repository.
2. Create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. Commit your changes:
   ```bash
   git commit -am 'Add new feature'
   ```
4. Push to your branch:
   ```bash
   git push origin feature/your-feature-name
   ```
5. Open a Pull Request.


## Contact Information
For any questions, suggestions, or bug reports, please contact:

- **Name:** Mohamed Khaled Ahmed
- **Email:** mohamedkhaled47526@gmail.com
- **GitHub:** https://github.com/Mohamedkhaled687
