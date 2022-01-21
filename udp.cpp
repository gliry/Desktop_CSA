#include "udp.h"

UDP::UDP(QObject *parent) : QObject(parent)
{
// create a QUDP socket
    socket = new QUdpSocket(this);
    QHostAddress hostAddress;
    hostAddress.setAddress("169.254.57.9");
    // The most common way to use QUdpSocket class is
    // to bind to an address and port using bind()
    // bool QAbstractSocket::bind(const QHostAddress & address,
    //     quint16 port = 0, BindMode mode = DefaultForPlatform)
    socket->bind(hostAddress, 4999);
    socket->setSocketOption(QAbstractSocket :: ReceiveBufferSizeSocketOption, 2048 * 320 * 2 * 16 * 2);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void UDP::SendDataUDP(QVector<std::complex<double>> buffer)
{
    QHostAddress hostAddress;
    hostAddress.setAddress("169.254.57.9");
    QByteArray Data;
    int sended = 0;
    for (int i = 0; i < 20480; ++i)
    {
        for (int j = 0; j < 32; ++j)  // 32 * 8 byte * 2(real + imag) = 512 bytes
        {
            buf.append(buffer[i * 32 + j].real());     //пересчитать тут
            buf.append(buffer[i * 32 + j].imag());
            //qDebug() << buffer[i * 32 + j].real() <<buffer[i * 32 + j].imag();
        }
        Data.append(QByteArray::fromRawData(reinterpret_cast<const char*>(buf.data()),
                                                    64 *sizeof(double)));
        sended = socket->writeDatagram(Data, hostAddress, 4999);
        qDebug() << i << " - Sended: " << sended;
        buf.clear();
        Data.clear();
    }
}

QVector<std::complex<double>> UDP::readyRead()
{

    // when data comes in
    QByteArray buffer;
    QHostAddress hostAddress;
    hostAddress.setAddress("169.254.57.9");
    quint16 senderPort = 4999;

    // qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize,
    //                 QHostAddress * address = 0, quint16 * port = 0)
    // Receives a datagram no larger than maxSize bytes and stores it in data.
    // The sender's host address and port is stored in *address and *port
    // (unless the pointers are 0).
    int k = 0;
    int i = 0;
    while (!socket->hasPendingDatagrams()){
        qDebug() << "wait";
    }

    while (socket->hasPendingDatagrams())
    {
        QByteArray UDPBuffer;

         UDPBuffer.resize(socket->pendingDatagramSize());

         k = socket->readDatagram(UDPBuffer.data(),UDPBuffer.size(), &hostAddress, &senderPort);
         qDebug() << i << "  Received: " << k;
         ++i;
         buffer.append(UDPBuffer);
    }
    QDataStream stream(buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (int i = 0; i < 2048; ++i) // вычислить размер файла
    {
        for (int j = 0; j < 320; ++j)
        {
            double real, imag;
            stream >> real >> imag;
            std::complex<double> z(real, imag);
            buffer_receive_complex.append(z);
        }
    }


    qDebug() << "End";
    return buffer_receive_complex;
}
