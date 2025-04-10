#ifndef UDPPARSERWORKER_H
#define UDPPARSERWORKER_H

#include <QObject>
#include <QRunnable>
#include <QByteArray>
#include <atomic>

/**
 * @brief The UdpParserWorker class parses UDP datagrams in a thread pool
 *
 * This class is designed to run in a thread pool and efficiently parse UDP datagrams
 * without blocking the main thread or the receiver thread.
 */
class UdpParserWorker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit UdpParserWorker(bool debugMode = false, QObject *parent = nullptr);
    ~UdpParserWorker();

    // Implement QRunnable
    void run() override;

public slots:
    /**
     * @brief Parse a datagram
     * @param data The datagram data to parse
     */
    void parseDatagram(const QByteArray &data);

signals:
    /**
     * @brief Signal emitted when a datagram is successfully parsed
     */
    void datagramParsed(float speed, int rpm, int accPedal, int brakePedal,
                        double encoderAngle, float temperature, int batteryLevel,
                        double gpsLongitude, double gpsLatitude,
                        int speedFL, int speedFR, int speedBL, int speedBR);

    /**
     * @brief Signal emitted when an error occurs during parsing
     * @param error The error message
     */
    void errorOccurred(const QString &error);

private:
    bool m_debugMode;
    std::atomic<quint64> m_datagramsParsed;
};

#endif // UDPPARSERWORKER_H
