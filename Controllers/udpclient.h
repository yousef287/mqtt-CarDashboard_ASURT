#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QThreadPool>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAtomicInt>
#include <QTimer>
#include <QNetworkDatagram>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <atomic>
#include <memory>
#include <deque>

// Forward declarations for internal classes
class ThreadSafeQueue;
class DatagramReceiver;
class DatagramParser;

class UdpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(int rpm READ rpm WRITE setRpm NOTIFY rpmChanged)
    Q_PROPERTY(int accPedal READ accPedal WRITE setAccPedal NOTIFY accPedalChanged)
    Q_PROPERTY(int brakePedal READ brakePedal WRITE setBrakePedal NOTIFY brakePedalChanged)
    Q_PROPERTY(double encoderAngle READ encoderAngle WRITE setEncoderAngle NOTIFY encoderAngleChanged)
    Q_PROPERTY(float temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel WRITE setBatteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(double gpsLongitude READ gpsLongitude WRITE setGpsLongitude NOTIFY gpsLongitudeChanged)
    Q_PROPERTY(double gpsLatitude READ gpsLatitude WRITE setGpsLatitude NOTIFY gpsLatitudeChanged)
    Q_PROPERTY(int speedFL READ speedFL WRITE setSpeedFL NOTIFY speedFLChanged)
    Q_PROPERTY(int speedFR READ speedFR WRITE setSpeedFR NOTIFY speedFRChanged)
    Q_PROPERTY(int speedBL READ speedBL WRITE setSpeedBL NOTIFY speedBLChanged)
    Q_PROPERTY(int speedBR READ speedBR WRITE setSpeedBR NOTIFY speedBRChanged)

public:
    explicit UdpClient(QObject *parent = nullptr);
    ~UdpClient();

    // Bind UDP socket to the specified port.
    // Make it invokable so it can be called from QML
    Q_INVOKABLE bool start(quint16 port);
    Q_INVOKABLE bool stop();

    // Configure threading parameters (optional)
    Q_INVOKABLE void setParserThreadCount(int count);
    Q_INVOKABLE void setUpdateBatchSize(int size);
    Q_INVOKABLE void setQueueSize(int size);
    Q_INVOKABLE void setMaxPendingUpdates(int size);

    // Property getters and setters
    float speed() const;
    void setSpeed(float newSpeed);
    
    int rpm() const;
    void setRpm(int newRpm);
    
    int accPedal() const;
    void setAccPedal(int newAccPedal);
    
    int brakePedal() const;
    void setBrakePedal(int newBrakePedal);
    
    double encoderAngle() const;
    void setEncoderAngle(double newAngle);
    
    float temperature() const;
    void setTemperature(float newTemperature);
    
    int batteryLevel() const;
    void setBatteryLevel(int newBatteryLevel);
    
    double gpsLongitude() const;
    void setGpsLongitude(double newLongitude);
    
    double gpsLatitude() const;
    void setGpsLatitude(double newLatitude);
    
    int speedFL() const;
    void setSpeedFL(int newSpeedFL);
    
    int speedFR() const;
    void setSpeedFR(int newSpeedFR);
    
    int speedBL() const;
    void setSpeedBL(int newSpeedBL);
    
    int speedBR() const;
    void setSpeedBR(int newSpeedBR);

signals:
    void speedChanged(float newSpeed);
    void rpmChanged(int newRpm);
    void accPedalChanged(int newAccPedal);
    void brakePedalChanged(int newBrakePedal);
    void encoderAngleChanged(double newAngle);
    void temperatureChanged(float newTemperature);
    void batteryLevelChanged(int newBatteryLevel);
    void gpsLongitudeChanged(double newLongitude);
    void gpsLatitudeChanged(double newLatitude);
    void errorOccurred(const QString &error);
    void speedFLChanged(int newSpeedFL);
    void speedFRChanged(int newSpeedFR);
    void speedBLChanged(int newSpeedBL);
    void speedBRChanged(int newSpeedBR);

private slots:
    void handleParsedData(float speed, int rpm, int accPedal, int brakePedal, 
                         double encoderAngle, float temperature, int batteryLevel,
                         double gpsLongitude, double gpsLatitude,
                         int speedFL, int speedFR, int speedBL, int speedBR);
    void handleParsingError(const QString &error);
    void emitBatchedSignals();
    void monitorMemoryUsage();

private:
    // Thread-safe queue for passing datagrams between threads
    class ThreadSafeQueue {
    public:
        ThreadSafeQueue(int maxSize = 1000);
        ~ThreadSafeQueue();
        
        // Enqueue a datagram, returns false if queue is full
        bool enqueue(const QSharedPointer<QNetworkDatagram> &datagram);
        
        // Dequeue a datagram, waits up to timeoutMs if queue is empty
        // Returns true if successful, false on timeout
        bool dequeue(QSharedPointer<QNetworkDatagram> &datagram, int timeoutMs = 100);
        
        // Clear the queue
        void clear();
        
        // Get current size
        int size() const;
        
        // Check if queue is empty
        bool isEmpty() const;
        
        // Set shutdown flag to stop waiting threads
        void setShutdown(bool shutdown);
        
        // Get memory usage estimate in bytes
        qint64 memoryUsage() const;
        
    private:
        std::deque<QSharedPointer<QNetworkDatagram>> m_queue;
        mutable QMutex m_mutex;
        QWaitCondition m_condition;
        int m_maxSize;
        bool m_shutdown;
        std::atomic<qint64> m_memoryUsage;
    };

    // Datagram receiver thread
    class DatagramReceiver : public QThread {
    public:
        DatagramReceiver(ThreadSafeQueue *queue, QObject *parent = nullptr);
        ~DatagramReceiver();
        bool bind(const QHostAddress &address, quint16 port);
        void stop();
    protected:
        void run() override;
    private:
        QUdpSocket *m_socket;
        ThreadSafeQueue *m_queue;
        std::atomic<bool> m_running;
        QElapsedTimer m_throttleTimer;
        int m_droppedPackets;
        friend class UdpClient;
    };

    // Datagram parser worker
    class DatagramParser : public QObject, public QRunnable {
    public:
        DatagramParser(ThreadSafeQueue *queue, UdpClient *client);
        ~DatagramParser();
        void setRunning(bool running);
    protected:
        void run() override;
    private:
        ThreadSafeQueue *m_queue;
        UdpClient *m_client;
        std::atomic<bool> m_running;
        friend class UdpClient;
    };

    // Thread management
    DatagramReceiver *m_receiver;
    QThreadPool m_parserPool;
    QList<DatagramParser*> m_parsers;
    ThreadSafeQueue *m_queue;
    QTimer m_batchTimer;
    QTimer m_memoryMonitorTimer;
    
    // Configuration
    int m_parserThreadCount;
    int m_updateBatchSize;
    int m_queueSize;
    int m_batchInterval;
    int m_maxPendingUpdates;
    
    // Performance monitoring
    std::atomic<qint64> m_datagramsProcessed;
    std::atomic<qint64> m_datagramsDropped;
    std::atomic<qint64> m_memoryUsage;
    QElapsedTimer m_uptimeTimer;
    
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
    
    // Batched updates tracking
    struct PropertyUpdate {
        enum Type {
            Speed, Rpm, AccPedal, BrakePedal, EncoderAngle,
            Temperature, BatteryLevel, GpsLongitude, GpsLatitude,
            SpeedFL, SpeedFR, SpeedBL, SpeedBR
        };
        
        Type type;
        union {
            float floatValue;
            int intValue;
            double doubleValue;
        };
    };
    
    // Use a circular buffer for pending updates to prevent unbounded growth
    QList<PropertyUpdate> m_pendingUpdates;
    QMutex m_updateMutex;
    
    // Helper methods
    void initializeParsers();
    void cleanupParsers();
    void addPropertyUpdate(PropertyUpdate::Type type, float value);
    void addPropertyUpdate(PropertyUpdate::Type type, int value);
    void addPropertyUpdate(PropertyUpdate::Type type, double value);
    void limitPendingUpdates();
};

#endif // UDPCLIENT_H
