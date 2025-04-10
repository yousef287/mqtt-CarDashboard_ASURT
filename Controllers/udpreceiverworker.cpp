#include "udpreceiverworker.h"
#include <QDebug>
#include <QNetworkDatagram>


/* Runs in a dedicated thread and listens for UDP datagrams.
 * It emits a signal (datagramReceived) for each received datagram and tracks throughput statistics.
 */

UdpReceiverWorker::UdpReceiverWorker(QObject *parent)
    : QObject(parent),
    m_running(false),
    m_datagramsReceived(0),
    m_bytesReceived(0)
{
    m_socket = new QUdpSocket(this);

    // Connect readyRead signal to processPendingDatagrams slot
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpReceiverWorker::processPendingDatagrams);
}

UdpReceiverWorker::~UdpReceiverWorker()
{
    stopReceiving();
    m_socket->deleteLater();
}

void UdpReceiverWorker::initialize()
{
    m_statsTimer.start();
}

void UdpReceiverWorker::startReceiving(quint16 port)
{
    // Close socket if it's already open
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
    }

    // Bind socket to the specified port
    if (!m_socket->bind(QHostAddress::Any, port)) {
        emit errorOccurred(QString("Failed to bind UDP socket to port %1: %2")
                               .arg(port)
                               .arg(m_socket->errorString()));
        return;
    }

    m_running = true;
    m_datagramsReceived = 0;
    m_bytesReceived = 0;
    m_statsTimer.restart();
}

void UdpReceiverWorker::stopReceiving()
{
    m_running = false;
    m_socket->close();
}

void UdpReceiverWorker::processPendingDatagrams()
{
    // Process all pending datagrams
    while (m_socket->hasPendingDatagrams() && m_running) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QByteArray data = datagram.data();

        // Update statistics
        m_datagramsReceived++;
        m_bytesReceived += data.size();

        // Emit signal with datagram data
        emit datagramReceived(data);

        // Log statistics every 10 seconds
        if (m_statsTimer.elapsed() > 10000) {
            qDebug() << "UDP Receiver: Received" << m_datagramsReceived
                     << "datagrams (" << m_bytesReceived << "bytes) in the last"
                     << m_statsTimer.elapsed() / 1000.0 << "seconds";
            m_datagramsReceived = 0;
            m_bytesReceived = 0;
            m_statsTimer.restart();
        }
    }
}
