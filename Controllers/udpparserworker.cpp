#include "udpparserworker.h"
#include <QDebug>
#include <QStringList>
#include <QThread>

UdpParserWorker::UdpParserWorker(bool debugMode, QObject *parent)
    : QObject(parent),
    m_debugMode(debugMode),
    m_running(true),
    m_datagramsParsed(0)
{
    setAutoDelete(false);
}

UdpParserWorker::~UdpParserWorker()
{
    stop();

    // Clear the queue
    QMutexLocker locker(&m_queueMutex);
    m_queue.clear();
}

void UdpParserWorker::run()
{
    if (m_debugMode) {
        qDebug() << "Parser worker started in thread" << QThread::currentThreadId();
    }

    QByteArray datagram;

    while (m_running.load()) {
        // Get a datagram from the queue
        {
            QMutexLocker locker(&m_queueMutex);

            // Wait for data if queue is empty
            while (m_queue.isEmpty() && m_running.load()) {
                m_queueCondition.wait(&m_queueMutex, 100);
            }

            // Check if we should exit
            if (!m_running.load()) {
                break;
            }

            // Get the next datagram
            if (!m_queue.isEmpty()) {
                datagram = m_queue.dequeue();
            } else {
                continue;
            }
        }

        // Parse the datagram
        parseDatagram(datagram);
    }

    if (m_debugMode) {
        qDebug() << "Parser worker stopped in thread" << QThread::currentThreadId();
    }
}

void UdpParserWorker::queueDatagram(const QByteArray &data)
{
    QMutexLocker locker(&m_queueMutex);

    // Add datagram to queue
    m_queue.enqueue(data);

    // Wake up the worker thread
    m_queueCondition.wakeOne();
}

void UdpParserWorker::stop()
{
    m_running.store(false);

    // Wake up the worker thread
    QMutexLocker locker(&m_queueMutex);
    m_queueCondition.wakeAll();
}

void UdpParserWorker::parseDatagram(const QByteArray &data)
{
    try {
        // Convert datagram data to string and split by comma
        QString dataStr = QString::fromUtf8(data).trimmed();
        QStringList parts = dataStr.split(",");

        // Check if we have enough parts
        if (parts.size() >= 13) {
            bool okSpeed = false, okRpm = false, okAcc = false, okBrake = false;
            bool okAngle = false, okTemp = false, okBattery = false;
            bool okGpsLon = false, okGpsLat = false;
            bool okWSFL = false, okWSFR = false, okWSBL = false, okWSBR = false;

            // Parse values
            float speed = parts[0].toFloat(&okSpeed);
            int rpm = parts[1].toInt(&okRpm);
            int accPedal = parts[2].toInt(&okAcc);
            int brakePedal = parts[3].toInt(&okBrake);
            double encoderAngle = parts[4].toDouble(&okAngle);
            float temperature = parts[5].toFloat(&okTemp);
            int batteryLevel = parts[6].toInt(&okBattery);
            double gpsLongitude = parts[7].toDouble(&okGpsLon);
            double gpsLatitude = parts[8].toDouble(&okGpsLat);
            int speedFL = parts[9].toInt(&okWSFL);
            int speedFR = parts[10].toInt(&okWSFR);
            int speedBL = parts[11].toInt(&okWSBL);
            int speedBR = parts[12].toInt(&okWSBR);

            // Check if all values were parsed successfully
            if (okSpeed && okRpm && okAcc && okBrake && okAngle && okTemp &&
                okBattery && okGpsLon && okGpsLat &&
                okWSFL && okWSFR && okWSBL && okWSBR) {

                // Increment counter
                m_datagramsParsed++;

                // Emit signal with parsed data
                emit datagramParsed(
                    speed, rpm, accPedal, brakePedal,
                    encoderAngle, temperature, batteryLevel,
                    gpsLongitude, gpsLatitude,
                    speedFL, speedFR, speedBL, speedBR);

                // Log debug info occasionally
                if (m_debugMode && m_datagramsParsed % 1000 == 0) {
                    qDebug() << "Parser" << QThread::currentThreadId()
                    << "has processed" << m_datagramsParsed << "datagrams";
                }
            } else {
                emit errorOccurred("Failed to parse some values in datagram");
            }
        } else {
            emit errorOccurred(QString("Datagram has incorrect format (expected 13+ parts, got %1)").arg(parts.size()));
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Exception during parsing: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("Unknown exception during parsing");
    }
}
