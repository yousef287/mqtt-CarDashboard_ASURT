#include "udpclient.h"
#include <QStringList>
#include <QNetworkDatagram>
#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QProcess>

//=============================================================================
// ThreadSafeQueue Implementation
//=============================================================================

UdpClient::ThreadSafeQueue::ThreadSafeQueue(int maxSize)
    : m_maxSize(maxSize), m_shutdown(false), m_memoryUsage(0)
{
}

UdpClient::ThreadSafeQueue::~ThreadSafeQueue()
{
    setShutdown(true);
    clear();
}

bool UdpClient::ThreadSafeQueue::enqueue(const QSharedPointer<QNetworkDatagram> &datagram)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_shutdown) {
        return false;
    }
    
    if (m_queue.size() >= m_maxSize) {
        return false;
    }
    
    // Add to queue
    m_queue.push_back(datagram);
    
    // Update memory usage estimate (datagram size + overhead)
    m_memoryUsage.fetch_add(datagram->data().size() + sizeof(QNetworkDatagram) + sizeof(QSharedPointer<QNetworkDatagram>));
    
    m_condition.wakeOne();
    return true;
}

bool UdpClient::ThreadSafeQueue::dequeue(QSharedPointer<QNetworkDatagram> &datagram, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_queue.empty()) {
        if (m_shutdown) {
            return false;
        }
        
        // Wait for data or timeout
        bool result = m_condition.wait(&m_mutex, timeoutMs);
        if (!result || m_queue.empty()) {
            return false;
        }
    }
    
    // Get datagram from front of queue
    datagram = m_queue.front();
    m_queue.pop_front();
    
    // Update memory usage estimate
    if (datagram) {
        m_memoryUsage.fetch_sub(datagram->data().size() + sizeof(QNetworkDatagram) + sizeof(QSharedPointer<QNetworkDatagram>));
    }
    
    return true;
}

void UdpClient::ThreadSafeQueue::clear()
{
    QMutexLocker locker(&m_mutex);
    m_queue.clear();
    m_memoryUsage.store(0);
}

int UdpClient::ThreadSafeQueue::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_queue.size();
}

bool UdpClient::ThreadSafeQueue::isEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_queue.empty();
}

void UdpClient::ThreadSafeQueue::setShutdown(bool shutdown)
{
    QMutexLocker locker(&m_mutex);
    m_shutdown = shutdown;
    if (shutdown) {
        m_condition.wakeAll();
    }
}

qint64 UdpClient::ThreadSafeQueue::memoryUsage() const
{
    return m_memoryUsage.load();
}

//=============================================================================
// DatagramReceiver Implementation
//=============================================================================

UdpClient::DatagramReceiver::DatagramReceiver(ThreadSafeQueue *queue, QObject *parent)
    : QThread(parent), m_queue(queue), m_running(false), m_droppedPackets(0)
{
    m_socket = new QUdpSocket();
    m_throttleTimer.start();
}

UdpClient::DatagramReceiver::~DatagramReceiver()
{
    stop();
    wait();
    delete m_socket;
}

bool UdpClient::DatagramReceiver::bind(const QHostAddress &address, quint16 port)
{
    if (!m_socket->bind(address, port)) {
        return false;
    }
    return true;
}

void UdpClient::DatagramReceiver::stop()
{
    m_running = false;
    m_socket->close();
}

void UdpClient::DatagramReceiver::run()
{
    m_running = true;
    
    // Set high priority for this thread
    setPriority(QThread::HighPriority);
    
    QElapsedTimer timer;
    timer.start();
    
    while (m_running) {
        // Check for pending datagrams
        if (m_socket->hasPendingDatagrams()) {
            while (m_socket->hasPendingDatagrams() && m_running) {
                // Create a shared pointer to the datagram
                QSharedPointer<QNetworkDatagram> datagram = 
                    QSharedPointer<QNetworkDatagram>::create(m_socket->receiveDatagram());
                
                // Try to enqueue, if queue is full, drop the datagram
                if (!m_queue->enqueue(datagram)) {
                    m_droppedPackets++;
                    
                    // Log dropped packets periodically to avoid console spam
                    if (m_throttleTimer.elapsed() > 5000) {
                        qDebug() << "Queue full, dropped" << m_droppedPackets << "packets";
                        m_droppedPackets = 0;
                        m_throttleTimer.restart();
                    }
                }
            }
        } else {
            // No data, sleep a tiny bit to avoid CPU spinning
            msleep(1);
        }
    }
}

//=============================================================================
// DatagramParser Implementation
//=============================================================================

UdpClient::DatagramParser::DatagramParser(ThreadSafeQueue *queue, UdpClient *client)
    : QObject(nullptr), QRunnable(), m_queue(queue), m_client(client), m_running(true)
{
    setAutoDelete(false);
}

UdpClient::DatagramParser::~DatagramParser()
{
    m_running = false;
}

void UdpClient::DatagramParser::setRunning(bool running)
{
    m_running = running;
}

void UdpClient::DatagramParser::run()
{
    QSharedPointer<QNetworkDatagram> datagram;
    
    while (m_running) {
        try {
            // Try to get a datagram from the queue
            if (m_queue->dequeue(datagram, 100)) {
                if (!datagram) {
                    continue;
                }
                
                QString data = QString::fromUtf8(datagram->data()).trimmed();
                
                // Parse the datagram data
                QStringList parts = data.split(",");
                if (parts.size() >= 13) {
                    bool okSpeed = false, okRpm = false, okAcc = false, okBrake = false;
                    bool okAngle = false, okTemp = false, okBattery = false;
                    bool okGpsLon = false, okGpsLat = false;
                    bool okWSFL = false, okWSFR = false, okWSBL = false, okWSBR = false;
                    
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
                    
                    // Only emit if all values were parsed successfully
                    if (okSpeed && okRpm && okAcc && okBrake && okAngle && okTemp && 
                        okBattery && okGpsLon && okGpsLat && 
                        okWSFL && okWSFR && okWSBL && okWSBR) {
                        
                        // Use direct method call instead of dynamic_cast and signals
                        QMetaObject::invokeMethod(m_client, "handleParsedData", Qt::QueuedConnection,
                            Q_ARG(float, newSpeed), Q_ARG(int, newRpm), 
                            Q_ARG(int, newAcc), Q_ARG(int, newBrake),
                            Q_ARG(double, newAngle), Q_ARG(float, newTemp), 
                            Q_ARG(int, newBattery), Q_ARG(double, newGpsLon), 
                            Q_ARG(double, newGpsLat), Q_ARG(int, newWSFL), 
                            Q_ARG(int, newWSFR), Q_ARG(int, newWSBL), 
                            Q_ARG(int, newWSBR));
                    } else {
                        QMetaObject::invokeMethod(m_client, "handleParsingError", Qt::QueuedConnection,
                            Q_ARG(QString, "Failed to parse some values in datagram"));
                    }
                } else {
                    QMetaObject::invokeMethod(m_client, "handleParsingError", Qt::QueuedConnection,
                        Q_ARG(QString, "Datagram has incorrect format"));
                }
                
                // Clear the shared pointer to ensure proper memory cleanup
                datagram.clear();
            }
        } catch (const std::exception &e) {
            qWarning() << "Exception in parser thread:" << e.what();
            // Clear the shared pointer to ensure proper memory cleanup
            datagram.clear();
        } catch (...) {
            qWarning() << "Unknown exception in parser thread";
            // Clear the shared pointer to ensure proper memory cleanup
            datagram.clear();
        }
    }
}

//=============================================================================
// UdpClient Implementation
//=============================================================================

UdpClient::UdpClient(QObject *parent)
    : QObject(parent),
      m_parserThreadCount(QThread::idealThreadCount()),
      m_updateBatchSize(10),
      m_queueSize(1000),
      m_batchInterval(16), // ~60fps
      m_maxPendingUpdates(1000),
      m_datagramsProcessed(0),
      m_datagramsDropped(0),
      m_memoryUsage(0),
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
      m_speedBR(0)
{
    // Create the thread-safe queue
    m_queue = new ThreadSafeQueue(m_queueSize);
    
    // Create and configure the receiver thread
    m_receiver = new DatagramReceiver(m_queue, this);
    
    // Configure the parser thread pool
    m_parserPool.setMaxThreadCount(m_parserThreadCount);
    
    // Set up timer for batch updates
    connect(&m_batchTimer, &QTimer::timeout, this, &UdpClient::emitBatchedSignals);
    m_batchTimer.setInterval(m_batchInterval);
    
    // Set up timer for memory monitoring
    connect(&m_memoryMonitorTimer, &QTimer::timeout, this, &UdpClient::monitorMemoryUsage);
    m_memoryMonitorTimer.setInterval(5000); // Check every 5 seconds
    
    // Start the uptime timer
    m_uptimeTimer.start();
}

UdpClient::~UdpClient()
{
    stop();
    delete m_queue;
}

bool UdpClient::start(quint16 port)
{
    // Stop if already running
    stop();
    
    // Bind the receiver socket
    if (!m_receiver->bind(QHostAddress::Any, port)) {
        emit errorOccurred("Failed to bind UDP socket");
        return false;
    }
    
    // Initialize parser threads
    initializeParsers();
    
    // Start the receiver thread
    m_receiver->start();
    
    // Start the timers
    m_batchTimer.start();
    m_memoryMonitorTimer.start();
    
    qDebug() << "Threaded UDP Client started on port" << port 
             << "with" << m_parserThreadCount << "parser threads";
    
    return true;
}

bool UdpClient::stop()
{
    // Stop the timers
    m_batchTimer.stop();
    m_memoryMonitorTimer.stop();
    
    // Stop the receiver thread
    if (m_receiver->isRunning()) {
        m_receiver->stop();
        m_receiver->wait();
    }
    
    // Clean up parser threads
    cleanupParsers();
    
    // Clear the queue
    m_queue->setShutdown(true);
    m_queue->clear();
    m_queue->setShutdown(false);
    
    // Clear pending updates
    QMutexLocker locker(&m_updateMutex);
    m_pendingUpdates.clear();
    
    qDebug() << "Threaded UDP Client stopped";
    
    return true;
}

void UdpClient::setParserThreadCount(int count)
{
    if (count > 0 && count <= QThread::idealThreadCount() * 2) {
        m_parserThreadCount = count;
        
        // If already running, restart with new thread count
        if (m_receiver->isRunning()) {
            quint16 port = m_receiver->m_socket->localPort();
            stop();
            start(port);
        }
    }
}

void UdpClient::setUpdateBatchSize(int size)
{
    if (size > 0) {
        m_updateBatchSize = size;
    }
}

void UdpClient::setQueueSize(int size)
{
    if (size > 0) {
        m_queueSize = size;
        
        // Create a new queue with the updated size
        ThreadSafeQueue *newQueue = new ThreadSafeQueue(m_queueSize);
        
        // If running, we need to restart
        if (m_receiver->isRunning()) {
            quint16 port = m_receiver->m_socket->localPort();
            stop();
            delete m_queue;
            m_queue = newQueue;
            start(port);
        } else {
            delete m_queue;
            m_queue = newQueue;
        }
    }
}

void UdpClient::setMaxPendingUpdates(int size)
{
    if (size > 0) {
        QMutexLocker locker(&m_updateMutex);
        m_maxPendingUpdates = size;
        limitPendingUpdates();
    }
}

float UdpClient::speed() const
{
    return m_speed.load();
}

void UdpClient::setSpeed(float newSpeed)
{
    if (qFuzzyCompare(m_speed.load(), newSpeed))
        return;
    m_speed.store(newSpeed);
    emit speedChanged(newSpeed);
}

int UdpClient::rpm() const
{
    return m_rpm.load();
}

void UdpClient::setRpm(int newRpm)
{
    if (m_rpm.load() == newRpm)
        return;
    m_rpm.store(newRpm);
    emit rpmChanged(newRpm);
}

int UdpClient::accPedal() const
{
    return m_accPedal.load();
}

void UdpClient::setAccPedal(int newAccPedal)
{
    if (m_accPedal.load() == newAccPedal)
        return;
    m_accPedal.store(newAccPedal);
    emit accPedalChanged(newAccPedal);
}

int UdpClient::brakePedal() const
{
    return m_brakePedal.load();
}

void UdpClient::setBrakePedal(int newBrakePedal)
{
    if (m_brakePedal.load() == newBrakePedal)
        return;
    m_brakePedal.store(newBrakePedal);
    emit brakePedalChanged(newBrakePedal);
}

double UdpClient::encoderAngle() const
{
    return m_encoderAngle.load();
}

void UdpClient::setEncoderAngle(double newAngle)
{
    if (qFuzzyCompare(m_encoderAngle.load(), newAngle))
        return;
    m_encoderAngle.store(newAngle);
    emit encoderAngleChanged(newAngle);
}

float UdpClient::temperature() const
{
    return m_temperature.load();
}

void UdpClient::setTemperature(float newTemperature)
{
    if (qFuzzyCompare(m_temperature.load(), newTemperature))
        return;
    m_temperature.store(newTemperature);
    emit temperatureChanged(newTemperature);
}

int UdpClient::batteryLevel() const
{
    return m_batteryLevel.load();
}

void UdpClient::setBatteryLevel(int newBatteryLevel)
{
    if (m_batteryLevel.load() == newBatteryLevel)
        return;
    m_batteryLevel.store(newBatteryLevel);
    emit batteryLevelChanged(newBatteryLevel);
}

double UdpClient::gpsLongitude() const
{
    return m_gpsLongitude.load();
}

void UdpClient::setGpsLongitude(double newLongitude)
{
    if (qFuzzyCompare(m_gpsLongitude.load(), newLongitude))
        return;
    m_gpsLongitude.store(newLongitude);
    emit gpsLongitudeChanged(newLongitude);
}

double UdpClient::gpsLatitude() const
{
    return m_gpsLatitude.load();
}

void UdpClient::setGpsLatitude(double newLatitude)
{
    if (qFuzzyCompare(m_gpsLatitude.load(), newLatitude))
        return;
    m_gpsLatitude.store(newLatitude);
    emit gpsLatitudeChanged(newLatitude);
}

int UdpClient::speedFL() const
{
    return m_speedFL.load();
}

void UdpClient::setSpeedFL(int newSpeedFL)
{
    if (m_speedFL.load() == newSpeedFL)
        return;
    m_speedFL.store(newSpeedFL);
    emit speedFLChanged(newSpeedFL);
}

int UdpClient::speedFR() const
{
    return m_speedFR.load();
}

void UdpClient::setSpeedFR(int newSpeedFR)
{
    if (m_speedFR.load() == newSpeedFR)
        return;
    m_speedFR.store(newSpeedFR);
    emit speedFRChanged(newSpeedFR);
}

int UdpClient::speedBL() const
{
    return m_speedBL.load();
}

void UdpClient::setSpeedBL(int newSpeedBL)
{
    if (m_speedBL.load() == newSpeedBL)
        return;
    m_speedBL.store(newSpeedBL);
    emit speedBLChanged(newSpeedBL);
}

int UdpClient::speedBR() const
{
    return m_speedBR.load();
}

void UdpClient::setSpeedBR(int newSpeedBR)
{
    if (m_speedBR.load() == newSpeedBR)
        return;
    m_speedBR.store(newSpeedBR);
    emit speedBRChanged(newSpeedBR);
}

void UdpClient::handleParsedData(float speed, int rpm, int accPedal, int brakePedal, 
                                double encoderAngle, float temperature, int batteryLevel,
                                double gpsLongitude, double gpsLatitude,
                                int speedFL, int speedFR, int speedBL, int speedBR)
{
    // Increment processed count
    m_datagramsProcessed.fetch_add(1);
    
    // Update atomic values directly
    m_speed.store(speed);
    m_rpm.store(rpm);
    m_accPedal.store(accPedal);
    m_brakePedal.store(brakePedal);
    m_encoderAngle.store(encoderAngle);
    m_temperature.store(temperature);
    m_batteryLevel.store(batteryLevel);
    m_gpsLongitude.store(gpsLongitude);
    m_gpsLatitude.store(gpsLatitude);
    m_speedFL.store(speedFL);
    m_speedFR.store(speedFR);
    m_speedBL.store(speedBL);
    m_speedBR.store(speedBR);
    
    // Add to pending updates for batched signals
    QMutexLocker locker(&m_updateMutex);
    
    // Ensure we don't exceed the maximum number of pending updates
    limitPendingUpdates();
    
    // Add the updates
    addPropertyUpdate(PropertyUpdate::Speed, speed);
    addPropertyUpdate(PropertyUpdate::Rpm, rpm);
    addPropertyUpdate(PropertyUpdate::AccPedal, accPedal);
    addPropertyUpdate(PropertyUpdate::BrakePedal, brakePedal);
    addPropertyUpdate(PropertyUpdate::EncoderAngle, encoderAngle);
    addPropertyUpdate(PropertyUpdate::Temperature, temperature);
    addPropertyUpdate(PropertyUpdate::BatteryLevel, batteryLevel);
    addPropertyUpdate(PropertyUpdate::GpsLongitude, gpsLongitude);
    addPropertyUpdate(PropertyUpdate::GpsLatitude, gpsLatitude);
    addPropertyUpdate(PropertyUpdate::SpeedFL, speedFL);
    addPropertyUpdate(PropertyUpdate::SpeedFR, speedFR);
    addPropertyUpdate(PropertyUpdate::SpeedBL, speedBL);
    addPropertyUpdate(PropertyUpdate::SpeedBR, speedBR);
    
    // If we've reached the batch size, emit signals immediately
    if (m_pendingUpdates.size() >= m_updateBatchSize * 13) {
        QMetaObject::invokeMethod(this, "emitBatchedSignals", Qt::QueuedConnection);
    }
}

void UdpClient::handleParsingError(const QString &error)
{
    qDebug() << "Parsing error:" << error;
}

void UdpClient::emitBatchedSignals()
{
    QMutexLocker locker(&m_updateMutex);
    
    if (m_pendingUpdates.isEmpty()) {
        return;
    }
    
    // Track which properties have been updated to emit only the latest value
    bool speedUpdated = false;
    bool rpmUpdated = false;
    bool accPedalUpdated = false;
    bool brakePedalUpdated = false;
    bool encoderAngleUpdated = false;
    bool temperatureUpdated = false;
    bool batteryLevelUpdated = false;
    bool gpsLongitudeUpdated = false;
    bool gpsLatitudeUpdated = false;
    bool speedFLUpdated = false;
    bool speedFRUpdated = false;
    bool speedBLUpdated = false;
    bool speedBRUpdated = false;
    
    // Get the latest values
    float latestSpeed = m_speed.load();
    int latestRpm = m_rpm.load();
    int latestAccPedal = m_accPedal.load();
    int latestBrakePedal = m_brakePedal.load();
    double latestEncoderAngle = m_encoderAngle.load();
    float latestTemperature = m_temperature.load();
    int latestBatteryLevel = m_batteryLevel.load();
    double latestGpsLongitude = m_gpsLongitude.load();
    double latestGpsLatitude = m_gpsLatitude.load();
    int latestSpeedFL = m_speedFL.load();
    int latestSpeedFR = m_speedFR.load();
    int latestSpeedBL = m_speedBL.load();
    int latestSpeedBR = m_speedBR.load();
    
    // Process all pending updates
    for (const PropertyUpdate &update : m_pendingUpdates) {
        switch (update.type) {
            case PropertyUpdate::Speed:
                speedUpdated = true;
                break;
            case PropertyUpdate::Rpm:
                rpmUpdated = true;
                break;
            case PropertyUpdate::AccPedal:
                accPedalUpdated = true;
                break;
            case PropertyUpdate::BrakePedal:
                brakePedalUpdated = true;
                break;
            case PropertyUpdate::EncoderAngle:
                encoderAngleUpdated = true;
                break;
            case PropertyUpdate::Temperature:
                temperatureUpdated = true;
                break;
            case PropertyUpdate::BatteryLevel:
                batteryLevelUpdated = true;
                break;
            case PropertyUpdate::GpsLongitude:
                gpsLongitudeUpdated = true;
                break;
            case PropertyUpdate::GpsLatitude:
                gpsLatitudeUpdated = true;
                break;
            case PropertyUpdate::SpeedFL:
                speedFLUpdated = true;
                break;
            case PropertyUpdate::SpeedFR:
                speedFRUpdated = true;
                break;
            case PropertyUpdate::SpeedBL:
                speedBLUpdated = true;
                break;
            case PropertyUpdate::SpeedBR:
                speedBRUpdated = true;
                break;
        }
    }
    
    // Clear pending updates
    m_pendingUpdates.clear();
    
    // Release the lock before emitting signals
    locker.unlock();
    
    // Emit signals for updated properties
    if (speedUpdated) {
        emit speedChanged(latestSpeed);
    }
    if (rpmUpdated) {
        emit rpmChanged(latestRpm);
    }
    if (accPedalUpdated) {
        emit accPedalChanged(latestAccPedal);
    }
    if (brakePedalUpdated) {
        emit brakePedalChanged(latestBrakePedal);
    }
    if (encoderAngleUpdated) {
        emit encoderAngleChanged(latestEncoderAngle);
    }
    if (temperatureUpdated) {
        emit temperatureChanged(latestTemperature);
    }
    if (batteryLevelUpdated) {
        emit batteryLevelChanged(latestBatteryLevel);
    }
    if (gpsLongitudeUpdated) {
        emit gpsLongitudeChanged(latestGpsLongitude);
    }
    if (gpsLatitudeUpdated) {
        emit gpsLatitudeChanged(latestGpsLatitude);
    }
    if (speedFLUpdated) {
        emit speedFLChanged(latestSpeedFL);
    }
    if (speedFRUpdated) {
        emit speedFRChanged(latestSpeedFR);
    }
    if (speedBLUpdated) {
        emit speedBLChanged(latestSpeedBL);
    }
    if (speedBRUpdated) {
        emit speedBRChanged(latestSpeedBR);
    }
}

void UdpClient::monitorMemoryUsage()
{
    // Get memory usage from queue
    qint64 queueMemory = m_queue->memoryUsage();
    
    // Estimate memory usage for pending updates
    qint64 pendingUpdatesMemory = 0;
    {
        QMutexLocker locker(&m_updateMutex);
        pendingUpdatesMemory = m_pendingUpdates.size() * sizeof(PropertyUpdate);
    }
    
    // Update total memory usage
    m_memoryUsage.store(queueMemory + pendingUpdatesMemory);
    
    // Log memory usage periodically
    static int logCounter = 0;
    if (++logCounter >= 12) { // Log every minute (12 * 5 seconds)
        logCounter = 0;
        
        // Get process memory info
        QProcess process;
        process.start("ps", QStringList() << "-p" << QString::number(QCoreApplication::applicationPid()) << "-o" << "rss=");
        process.waitForFinished();
        QString rss = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        
        qDebug() << "Memory usage - Queue:" << (queueMemory / 1024) << "KB, "
                 << "Pending updates:" << (pendingUpdatesMemory / 1024) << "KB, "
                 << "Process RSS:" << rss << "KB, "
                 << "Uptime:" << (m_uptimeTimer.elapsed() / 1000) << "seconds, "
                 << "Processed:" << m_datagramsProcessed.load() << "datagrams";
    }
    
    // Check for excessive memory usage and take action if needed
    if (queueMemory > 100 * 1024 * 1024) { // More than 100MB in queue
        qWarning() << "Excessive memory usage detected in queue, clearing...";
        m_queue->clear();
    }
}

void UdpClient::initializeParsers()
{
    // Create parser instances
    for (int i = 0; i < m_parserThreadCount; ++i) {
        DatagramParser *parser = new DatagramParser(m_queue, this);
        m_parsers.append(parser);
        m_parserPool.start(parser);
    }
}

void UdpClient::cleanupParsers()
{
    // Stop all parsers
    for (DatagramParser *parser : m_parsers) {
        parser->setRunning(false);
    }
    
    // Wait for all parsers to finish
    m_parserPool.waitForDone();
    
    // Delete all parsers
    qDeleteAll(m_parsers);
    m_parsers.clear();
}

void UdpClient::addPropertyUpdate(PropertyUpdate::Type type, float value)
{
    PropertyUpdate update;
    update.type = type;
    update.floatValue = value;
    m_pendingUpdates.append(update);
}

void UdpClient::addPropertyUpdate(PropertyUpdate::Type type, int value)
{
    PropertyUpdate update;
    update.type = type;
    update.intValue = value;
    m_pendingUpdates.append(update);
}

void UdpClient::addPropertyUpdate(PropertyUpdate::Type type, double value)
{
    PropertyUpdate update;
    update.type = type;
    update.doubleValue = value;
    m_pendingUpdates.append(update);
}

void UdpClient::limitPendingUpdates()
{
    // If we exceed the maximum number of pending updates, remove oldest ones
    while (m_pendingUpdates.size() > m_maxPendingUpdates) {
        m_pendingUpdates.removeFirst();
        m_datagramsDropped.fetch_add(1);
    }
}
