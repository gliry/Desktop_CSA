#ifndef UDP_H
#define UDP_H

#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include <complex>
#include <QDataStream>

class UDP : public QObject
{
    Q_OBJECT
public:
    explicit UDP(QObject *parent = nullptr);
    void SendDataUDP(QVector<std::complex<double>> buffer);
    QVector<std::complex<double>> buffer_receive_complex;
    QVector<std::complex<double>> readyRead();


private:
    QUdpSocket *socket;
    QVector<double> buf;
    QVector<double> buffer_receive;


};

#endif // UDP_H
