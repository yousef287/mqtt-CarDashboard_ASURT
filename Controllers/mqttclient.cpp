#include "mqttclient.h"
#include "udpparserworker.h"
#include <QDebug>

static const char *MQTT_HOST = "5aeaff002e7c423299c2d92361292d54.s1.eu.hivemq.cloud";
static const int   MQTT_PORT = 8883;
static const char *MQTT_USER = "yousef";
static const char *MQTT_PASS = "Yousef123";
static const char *MQTT_TOPIC = "com/yousef/esp32/data";

MqttClient::MqttClient(QObject *parent)
    : QObject(parent),
      m_client(nullptr),
      m_nextParserIndex(0),
      m_parserThreadCount(QThread::idealThreadCount()),
      m_debugMode(true),
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
      m_lateralG(0.0),
      m_longitudinalG(0.0)
{
    m_parserPool.setMaxThreadCount(m_parserThreadCount);
}

MqttClient::~MqttClient()
{
    stop();
}

bool MqttClient::start()
{
    stop();
    initializeParsers();

    m_client = new QMqttClient();
    m_client->setHostname(QString::fromLatin1(MQTT_HOST));
    m_client->setPort(MQTT_PORT);
    m_client->setUsername(QString::fromLatin1(MQTT_USER));
    m_client->setPassword(QString::fromLatin1(MQTT_PASS));
    m_client->moveToThread(&m_clientThread);

    connect(&m_clientThread, &QThread::started, [this]() {
        if (m_debugMode)
            qDebug() << "Connecting to MQTT broker";
        m_client->connectToHostEncrypted();
    });
    connect(m_client, &QMqttClient::connected, this, &MqttClient::onConnected);
    connect(m_client, &QMqttClient::messageReceived, this, &MqttClient::handleMessage);
    connect(&m_clientThread, &QThread::finished, m_client, &QObject::deleteLater);
    connect(m_client, &QMqttClient::disconnected, [this]() {
        if (m_debugMode)
            qDebug() << "MQTT disconnected";
    });

    m_clientThread.start();
    return true;
}

bool MqttClient::stop()
{
    if (m_client) {
        QMetaObject::invokeMethod(m_client, "disconnectFromHost", Qt::QueuedConnection);
    }
    if (m_clientThread.isRunning()) {
        m_clientThread.quit();
        m_clientThread.wait();
    }
    m_client = nullptr;
    cleanupParsers();
    return true;
}

void MqttClient::setParserThreadCount(int count)
{
    m_parserThreadCount = count > 0 ? count : QThread::idealThreadCount();
    m_parserPool.setMaxThreadCount(m_parserThreadCount);
}

void MqttClient::setDebugMode(bool enabled)
{
    m_debugMode = enabled;
}

void MqttClient::onConnected()
{
    if (m_debugMode)
        qDebug() << "MQTT connected";
    QMqttSubscription *sub = m_client->subscribe(QString::fromLatin1(MQTT_TOPIC));
    if (!sub && m_debugMode)
        qDebug() << "Failed to subscribe";
}

void MqttClient::handleMessage(const QByteArray &message)
{
    if (m_parsers.isEmpty())
        return;
    UdpParserWorker *parser = m_parsers[m_nextParserIndex];
    parser->queueDatagram(message);
    m_nextParserIndex = (m_nextParserIndex + 1) % m_parsers.size();
}

void MqttClient::handleParsedData(float speed, int rpm, int accPedal, int brakePedal,
                                  double encoderAngle, float temperature, int batteryLevel,
                                  double gpsLongitude, double gpsLatitude,
                                  int speedFL, int speedFR, int speedBL, int speedBR,
                                  double lateralG, double longitudinalG)
{
    if (!qFuzzyCompare(m_speed.load(), speed)) {
        m_speed.store(speed);
        emit speedChanged(speed);
    }
    if (m_rpm.load() != rpm) {
        m_rpm.store(rpm);
        emit rpmChanged(rpm);
    }
    if (m_accPedal.load() != accPedal) {
        m_accPedal.store(accPedal);
        emit accPedalChanged(accPedal);
    }
    if (m_brakePedal.load() != brakePedal) {
        m_brakePedal.store(brakePedal);
        emit brakePedalChanged(brakePedal);
    }
    if (!qFuzzyCompare(m_encoderAngle.load(), encoderAngle)) {
        m_encoderAngle.store(encoderAngle);
        emit encoderAngleChanged(encoderAngle);
    }
    if (!qFuzzyCompare(m_temperature.load(), temperature)) {
        m_temperature.store(temperature);
        emit temperatureChanged(temperature);
    }
    if (m_batteryLevel.load() != batteryLevel) {
        m_batteryLevel.store(batteryLevel);
        emit batteryLevelChanged(batteryLevel);
    }
    if (!qFuzzyCompare(m_gpsLongitude.load(), gpsLongitude)) {
        m_gpsLongitude.store(gpsLongitude);
        emit gpsLongitudeChanged(gpsLongitude);
    }
    if (!qFuzzyCompare(m_gpsLatitude.load(), gpsLatitude)) {
        m_gpsLatitude.store(gpsLatitude);
        emit gpsLatitudeChanged(gpsLatitude);
    }
    if (m_speedFL.load() != speedFL) {
        m_speedFL.store(speedFL);
        emit speedFLChanged(speedFL);
    }
    if (m_speedFR.load() != speedFR) {
        m_speedFR.store(speedFR);
        emit speedFRChanged(speedFR);
    }
    if (m_speedBL.load() != speedBL) {
        m_speedBL.store(speedBL);
        emit speedBLChanged(speedBL);
    }
    if (m_speedBR.load() != speedBR) {
        m_speedBR.store(speedBR);
        emit speedBRChanged(speedBR);
    }
    if (!qFuzzyCompare(m_lateralG.load(), lateralG)) {
        m_lateralG.store(lateralG);
        emit lateralGChanged(lateralG);
    }
    if (!qFuzzyCompare(m_longitudinalG.load(), longitudinalG)) {
        m_longitudinalG.store(longitudinalG);
        emit longitudinalGChanged(longitudinalG);
    }
}

void MqttClient::handleError(const QString &error)
{
    if (m_debugMode)
        qDebug() << "MQTT error:" << error;
    emit errorOccurred(error);
}

void MqttClient::initializeParsers()
{
    for (int i = 0; i < m_parserThreadCount; ++i) {
        UdpParserWorker *parser = new UdpParserWorker(m_debugMode);
        connect(parser, &UdpParserWorker::datagramParsed, this, &MqttClient::handleParsedData, Qt::QueuedConnection);
        connect(parser, &UdpParserWorker::errorOccurred, this, &MqttClient::handleError, Qt::QueuedConnection);
        m_parsers.append(parser);
        m_parserPool.start(parser);
    }
    m_nextParserIndex = 0;
}

void MqttClient::cleanupParsers()
{
    for (UdpParserWorker *parser : m_parsers) {
        parser->stop();
    }
    m_parserPool.waitForDone();
    for (UdpParserWorker *parser : m_parsers) {
        disconnect(parser, nullptr, this, nullptr);
    }
    qDeleteAll(m_parsers);
    m_parsers.clear();
}

