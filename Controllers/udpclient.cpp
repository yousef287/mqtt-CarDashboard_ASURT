#include "UdpClient.h"
#include <QStringList>
#include <QNetworkDatagram>
#include <QDebug>

UdpClient::UdpClient(QObject *parent)
    : QObject(parent),
    m_speed(0.0f),
    m_rpm(0),
    m_accPedal(0),
    m_brakePedal(0),
    m_encoderAngle(0.0),
    m_temperature(0.0f),
    m_batteryLevel(0),
    m_gpsLongitude(0.0),
    m_gpsLatitude(0.0),
    m_speedFL(0),
    m_speedFR(0),
    m_speedBL(0),
    m_speedBR(0),
    m_yawAngle(0.0),
    m_pitchAngle(0.0),
    m_rollAngle(0.0)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpClient::processPendingDatagrams);
}

bool UdpClient::start(quint16 port)
{
    if (!m_socket->bind(QHostAddress::Any, port)) {
        emit errorOccurred("Failed to bind UDP socket");
        return false;
    }
#ifdef QT_DEBUG
    qDebug() << "UDP Client listening on port" << port;
#endif
    return true;
}

bool UdpClient::stop()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
#ifdef QT_DEBUG
        qDebug() << "UDP Client socket closed";
#endif
    }
    return true;
}

float UdpClient::speed() const { return m_speed; }
void UdpClient::setSpeed(float newSpeed)
{
    if (qFuzzyCompare(m_speed, newSpeed))
        return;
    m_speed = newSpeed;
    emit speedChanged(m_speed);
}

int UdpClient::rpm() const { return m_rpm; }
void UdpClient::setRpm(int newRpm)
{
    if (m_rpm == newRpm)
        return;
    m_rpm = newRpm;
    emit rpmChanged(m_rpm);
}

int UdpClient::accPedal() const { return m_accPedal; }
void UdpClient::setAccPedal(int newAccPedal)
{
    if (m_accPedal == newAccPedal)
        return;
    m_accPedal = newAccPedal;
    emit accPedalChanged(m_accPedal);
}

int UdpClient::brakePedal() const { return m_brakePedal; }
void UdpClient::setBrakePedal(int newBrakePedal)
{
    if (m_brakePedal == newBrakePedal)
        return;
    m_brakePedal = newBrakePedal;
    emit brakePedalChanged(m_brakePedal);
}

double UdpClient::encoderAngle() const { return m_encoderAngle; }
void UdpClient::setEncoderAngle(double newAngle)
{
    if (qFuzzyCompare(m_encoderAngle, newAngle))
        return;
    m_encoderAngle = newAngle;
    emit encoderAngleChanged(m_encoderAngle);
}

float UdpClient::temperature() const { return m_temperature; }
void UdpClient::setTemperature(float newTemperature)
{
    if (qFuzzyCompare(m_temperature, newTemperature))
        return;
    m_temperature = newTemperature;
    emit temperatureChanged(m_temperature);
}

int UdpClient::batteryLevel() const { return m_batteryLevel; }
void UdpClient::setBatteryLevel(int newBatteryLevel)
{
    if (m_batteryLevel == newBatteryLevel)
        return;
    m_batteryLevel = newBatteryLevel;
    emit batteryLevelChanged(m_batteryLevel);
}

double UdpClient::gpsLongitude() const { return m_gpsLongitude; }
void UdpClient::setGpsLongitude(double newLongitude)
{
    if (qFuzzyCompare(m_gpsLongitude, newLongitude))
        return;
    m_gpsLongitude = newLongitude;
    emit gpsLongitudeChanged(m_gpsLongitude);
}

double UdpClient::gpsLatitude() const { return m_gpsLatitude; }
void UdpClient::setGpsLatitude(double newLatitude)
{
    if (qFuzzyCompare(m_gpsLatitude, newLatitude))
        return;
    m_gpsLatitude = newLatitude;
    emit gpsLatitudeChanged(m_gpsLatitude);
}

int UdpClient::speedFL() const { return m_speedFL; }
void UdpClient::setSpeedFL(int newSpeedFL)
{
    if (m_speedFL == newSpeedFL)
        return;
    m_speedFL = newSpeedFL;
    emit speedFLChanged(m_speedFL);
}

int UdpClient::speedFR() const { return m_speedFR; }
void UdpClient::setSpeedFR(int newSpeedFR)
{
    if (m_speedFR == newSpeedFR)
        return;
    m_speedFR = newSpeedFR;
    emit speedFRChanged(m_speedFR);
}

int UdpClient::speedBL() const { return m_speedBL; }
void UdpClient::setSpeedBL(int newSpeedBL)
{
    if (m_speedBL == newSpeedBL)
        return;
    m_speedBL = newSpeedBL;
    emit speedBLChanged(m_speedBL);
}

int UdpClient::speedBR() const { return m_speedBR; }
void UdpClient::setSpeedBR(int newSpeedBR)
{
    if (m_speedBR == newSpeedBR)
        return;
    m_speedBR = newSpeedBR;
    emit speedBRChanged(m_speedBR);
}

// Getter and Setter for yawAngle
double UdpClient::yawAngle() const { return m_yawAngle; }
void UdpClient::setYawAngle(double newYawAngle)
{
    if (qFuzzyCompare(m_yawAngle, newYawAngle))
        return;
    m_yawAngle = newYawAngle;
    emit yawAngleChanged(m_yawAngle);
}

// Getter and Setter for pitchAngle
double UdpClient::pitchAngle() const { return m_pitchAngle; }
void UdpClient::setPitchAngle(double newPitchAngle)
{
    if (qFuzzyCompare(m_pitchAngle, newPitchAngle))
        return;
    m_pitchAngle = newPitchAngle;
    emit pitchAngleChanged(m_pitchAngle);
}

// Getter and Setter for rollAngle
double UdpClient::rollAngle() const { return m_rollAngle; }
void UdpClient::setRollAngle(double newRollAngle)
{
    if (qFuzzyCompare(m_rollAngle, newRollAngle))
        return;
    m_rollAngle = newRollAngle;
    emit rollAngleChanged(m_rollAngle);
}

void UdpClient::processPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QString data = QString::fromUtf8(datagram.data()).trimmed();
        // Expected format:
        // "speed,rpm,accPedal,brakePedal,encoderAngle,temperature,batteryLevel,
        //  gpsLongitude,gpsLatitude,speedFL,speedFR,speedBL,speedBR,yawAngle,pitchAngle,rollAngle"
        QStringList parts = data.split(",");
        if (parts.size() >= 16) {
            bool okSpeed = false, okRpm = false, okAcc = false, okBrake = false;
            bool okAngle = false, okTemp = false, okBattery = false;
            bool okGpsLon = false, okGpsLat = false;
            bool okWSFL = false, okWSFR = false, okWSBL = false, okWSBR = false;
            bool okYaw = false, okPitch = false, okRoll = false;

            float newSpeed = parts[0].toFloat(&okSpeed);
            int newRpm = parts[1].toInt(&okRpm);
            int newAcc = parts[2].toInt(&okAcc);
            int newBrake = parts[3].toInt(&okBrake);
            double newAngle = parts[4].toDouble(&okAngle);
            float newTemp = parts[5].toFloat(&okTemp);
            int newBattery = parts[6].toInt(&okBattery);
            double newGpsLon = parts[7].toDouble(&okGpsLon);
            double newGpsLat = parts[8].toDouble(&okGpsLat);
            int newWSFL = parts[9].toInt(&okWSFL);
            int newWSFR = parts[10].toInt(&okWSFR);
            int newWSBL = parts[11].toInt(&okWSBL);
            int newWSBR = parts[12].toInt(&okWSBR);
            double newYaw = parts[13].toDouble(&okYaw);
            double newPitch = parts[14].toDouble(&okPitch);
            double newRoll = parts[15].toDouble(&okRoll);

            if (okSpeed) {
                setSpeed(newSpeed);
            }
            if (okRpm) {
                setRpm(newRpm);
            }
            if (okAcc) {
                setAccPedal(newAcc);
            }
            if (okBrake) {
                setBrakePedal(newBrake);
            }
            if (okAngle) {
                setEncoderAngle(newAngle);
            }
            if (okTemp) {
                setTemperature(newTemp);
            }
            if (okBattery) {
                setBatteryLevel(newBattery);
            }
            if (okGpsLon) {
                setGpsLongitude(newGpsLon);
            }
            if (okGpsLat) {
                setGpsLatitude(newGpsLat);
            }
            if (okWSFL) {
                setSpeedFL(newWSFL);
            }
            if (okWSFR) {
                setSpeedFR(newWSFR);
            }
            if (okWSBL) {
                setSpeedBL(newWSBL);
            }
            if (okWSBR) {
                setSpeedBR(newWSBR);
            }
            if (okYaw) {
                setYawAngle(newYaw);
            }
            if (okPitch) {
                setPitchAngle(newPitch);
            }
            if (okRoll) {
                setRollAngle(newRoll);
            }
        }
    }
}
