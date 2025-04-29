#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QThreadPool>
#include <QAtomicInt>
#include <QNetworkDatagram>
#include <atomic>

// Forward declarations
class UdpReceiverWorker;
class UdpParserWorker;

/**
 * @brief The UdpClient class provides a high-performance UDP client for receiving and parsing datagrams
 *
 * This class uses a simplified threading model with proper thread pool utilization for maximum performance.
 * It maintains the same public API as the original implementation while significantly reducing complexity.
 */
class UdpClient : public QObject
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
    explicit UdpClient(QObject *parent = nullptr); // Initialize the Client , its threads and workers.
    ~UdpClient();

    /**
     * @brief Start the UDP client on the specified port
     * @param port The UDP port to listen on
     * @return True if successful, false otherwise
     */
    Q_INVOKABLE bool start(quint16 port);

    /**
     * @brief Stop the UDP client
     * @return True if successful, false otherwise
     */
    Q_INVOKABLE bool stop();

    /**
     * @brief Configure the number of parser threads
     * @param count The number of parser threads to use (default: number of CPU cores)
     */
    Q_INVOKABLE void setParserThreadCount(int count);

    /**
     * @brief Enable or disable debug mode
     * @param enabled Whether debug mode should be enabled
     */
    Q_INVOKABLE void setDebugMode(bool enabled);

    // Property getters
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
    // Property change signals
    void speedChanged(float newSpeed);
    void rpmChanged(int newRpm);
    void accPedalChanged(int newAccPedal);
    void brakePedalChanged(int newBrakePedal);
    void encoderAngleChanged(double newAngle);
    void temperatureChanged(float newTemperature);
    void batteryLevelChanged(int newBatteryLevel);
    void gpsLongitudeChanged(double newLongitude);
    void gpsLatitudeChanged(double newLatitude);
    void speedFLChanged(int newSpeedFL);
    void speedFRChanged(int newSpeedFR);
    void speedBLChanged(int newSpeedBL);
    void speedBRChanged(int newSpeedBR);
    void lateralGChanged(double newLateralG);
    void longitudinalGChanged(double newLongitudinalG);

    // Error signal
    void errorOccurred(const QString &error);

    // Internal signals for worker communication
    void startReceiving(quint16 port);
    void stopReceiving();

private slots:
    void handleParsedData(float speed, int rpm, int accPedal, int brakePedal,
                                     double encoderAngle, float temperature, int batteryLevel,
                                     double gpsLongitude, double gpsLatitude,
                                     int speedFL, int speedFR, int speedBL, int speedBR,
                          double lateralG, double longitudinalG);

    void handleError(const QString &error); // Handles error messages from workers.

    void handleDatagramReceived(const QByteArray &data); // Receives raw datagrams from the receiver worker and dispatches them to parser workers.

private:
    // Worker threads
    QThread m_receiverThread;            // Dedicated thread for the receiver worker
    UdpReceiverWorker *m_receiverWorker; // The worker that listens to the UDP datagrams

    QThreadPool m_parserPool;           // A thread pool to run multiple parsers workers concurrently
    QList<UdpParserWorker *> m_parsers; // list of  parser worker objects
    int m_nextParserIndex;              // Used to cycle through parser workers in a round-robin fashion, distributing incoming datagrams among multiple parsers.

    // Configuration
    int m_parserThreadCount;
    bool m_debugMode;

    // Performance tracking
    std::atomic<qint64> m_datagramsProcessed;
    std::atomic<qint64> m_datagramsDropped;

    // Data storage with atomic access
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

    // Helper methods
    void initializeParsers();
    void cleanupParsers();
};

#endif // UDPCLIENT_H
