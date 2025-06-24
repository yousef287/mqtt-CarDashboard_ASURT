#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QMqttClient>
#include <QThread>
#include <QThreadPool>
#include <atomic>

class UdpParserWorker;

class MqttClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int rpm READ rpm NOTIFY rpmChanged)
    Q_PROPERTY(int accPedal READ accPedal NOTIFY accPedalChanged)
    Q_PROPERTY(int brakePedal READ brakePedal NOTIFY brakePedalChanged)
    Q_PROPERTY(double encoderAngle READ encoderAngle NOTIFY encoderAngleChanged)
    Q_PROPERTY(float temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(double gpsLongitude READ gpsLongitude NOTIFY gpsLongitudeChanged)
    Q_PROPERTY(double gpsLatitude READ gpsLatitude NOTIFY gpsLatitudeChanged)
    Q_PROPERTY(int speedFL READ speedFL NOTIFY speedFLChanged)
    Q_PROPERTY(int speedFR READ speedFR NOTIFY speedFRChanged)
    Q_PROPERTY(int speedBL READ speedBL NOTIFY speedBLChanged)
    Q_PROPERTY(int speedBR READ speedBR NOTIFY speedBRChanged)
    Q_PROPERTY(double lateralG READ lateralG NOTIFY lateralGChanged)
    Q_PROPERTY(double longitudinalG READ longitudinalG NOTIFY longitudinalGChanged)
public:
    explicit MqttClient(QObject *parent = nullptr);
    ~MqttClient();

    Q_INVOKABLE bool start();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE void setParserThreadCount(int count);
    Q_INVOKABLE void setDebugMode(bool enabled);

    float speed() const { return m_speed.load(); }
    int rpm() const { return m_rpm.load(); }
    int accPedal() const { return m_accPedal.load(); }
    int brakePedal() const { return m_brakePedal.load(); }
    double encoderAngle() const { return m_encoderAngle.load(); }
    float temperature() const { return m_temperature.load(); }
    int batteryLevel() const { return m_batteryLevel.load(); }
    double gpsLongitude() const { return m_gpsLongitude.load(); }
    double gpsLatitude() const { return m_gpsLatitude.load(); }
    int speedFL() const { return m_speedFL.load(); }
    int speedFR() const { return m_speedFR.load(); }
    int speedBL() const { return m_speedBL.load(); }
    int speedBR() const { return m_speedBR.load(); }
    double lateralG() const { return m_lateralG.load(); }
    double longitudinalG() const { return m_longitudinalG.load(); }

signals:
    void speedChanged(float);
    void rpmChanged(int);
    void accPedalChanged(int);
    void brakePedalChanged(int);
    void encoderAngleChanged(double);
    void temperatureChanged(float);
    void batteryLevelChanged(int);
    void gpsLongitudeChanged(double);
    void gpsLatitudeChanged(double);
    void speedFLChanged(int);
    void speedFRChanged(int);
    void speedBLChanged(int);
    void speedBRChanged(int);
    void lateralGChanged(double);
    void longitudinalGChanged(double);
    void errorOccurred(const QString &error);

private slots:
    void handleMessage(const QByteArray &message);
    void handleParsedData(float speed, int rpm, int accPedal, int brakePedal,
                          double encoderAngle, float temperature, int batteryLevel,
                          double gpsLongitude, double gpsLatitude,
                          int speedFL, int speedFR, int speedBL, int speedBR,
                          double lateralG, double longitudinalG);
    void handleError(const QString &error);
    void onConnected();

private:
    void initializeParsers();
    void cleanupParsers();

    QMqttClient *m_client;
    QThread m_clientThread;
    QThreadPool m_parserPool;
    QList<UdpParserWorker *> m_parsers;
    int m_nextParserIndex;
    int m_parserThreadCount;
    bool m_debugMode;

    std::atomic<float> m_speed;
    std::atomic<int> m_rpm;
    std::atomic<int> m_accPedal;
    std::atomic<int> m_brakePedal;
    std::atomic<double> m_encoderAngle;
    std::atomic<float> m_temperature;
    std::atomic<int> m_batteryLevel;
    std::atomic<double> m_gpsLongitude;
    std::atomic<double> m_gpsLatitude;
    std::atomic<int> m_speedFL;
    std::atomic<int> m_speedFR;
    std::atomic<int> m_speedBL;
    std::atomic<int> m_speedBR;
    std::atomic<double> m_lateralG;
    std::atomic<double> m_longitudinalG;
};

#endif // MQTTCLIENT_H
