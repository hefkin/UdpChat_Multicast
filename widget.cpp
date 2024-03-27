#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    socketIn = new QUdpSocket(this);
    socketOut = new QUdpSocket(this);
    connect(socketIn, &QUdpSocket::readyRead, this, &Widget::slotReadyRead);

    portLineEdit = new QLineEdit();
    connectButton = new QPushButton("Connect");
    logTextEdit = new QTextEdit;
    messageLineEdit = new QLineEdit();
    sendButton = new QPushButton("Send");

    portLineEdit->setPlaceholderText("port");
    portLineEdit->setValidator(new QDoubleValidator(portLineEdit));
    messageLineEdit->setPlaceholderText("Enter a message:");
    connectButton->setCheckable(true);
    logTextEdit->setReadOnly(true);

    topLayout = new QHBoxLayout();
    topLayout->addWidget(portLineEdit);
    topLayout->addWidget(connectButton);

    bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(messageLineEdit);
    bottomLayout->addWidget(sendButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(logTextEdit);
    mainLayout->addLayout(bottomLayout);

    connect(sendButton, &QPushButton::clicked, this, &Widget::ClickedSlot);
    connect(connectButton, &QPushButton::toggled, this, &Widget::ToggleSlot);
}

Widget::~Widget()
{

}

void Widget::slotReadyRead()
{
    Data.resize(socketIn->pendingDatagramSize());
    qDebug() << "ready read!";
    QHostAddress *clientAddr = new QHostAddress();
    quint16 *cport = new quint16();
    socketIn->readDatagram(Data.data(), Data.size(), clientAddr, cport);

    if((localAddrs.contains(*clientAddr)) && (*cport == socketOut->localPort()))
    {
        return;
    }
    else
    {
        QString cip = clientAddr->toString();

        QDataStream in(&Data, QIODevice::ReadOnly);

        qDebug() << "reading size!";
        qint64 size = -1;
        if(in.device()->size() > sizeof(qint64))
        {
            in >> size;
        }
        else return;
        qDebug() << "message isnt empty!";
        if(in.device()->size() - sizeof(qint64) < size) return;
        QString str;
        in >> str;
        qDebug() << "message receivied!";
        printFromOther(str, cip, cport);
    }
}

void Widget::printTE(QString str)
{
    QString time_format = "dd.MM.yyyy HH:mm:ss";
    QDateTime cdt = QDateTime::currentDateTime();
    QString dt = cdt.toString(time_format);
    QString ipAddr = socketOut->localAddress().toString() + ":";
    QString portS = QString::number(socketOut->localPort());
    QString str1 = dt + " | " + ipAddr + portS + " | " + str;
    logTextEdit->append(str1);
}

void Widget::printFromOther(QString str, QString cip, quint16 *cport)
{
    QString time_format = "dd.MM.yyyy HH:mm:ss";
    QDateTime cdt = QDateTime::currentDateTime();
    QString dt = cdt.toString(time_format);
    QString portS = QString::number(*cport);
    QString str1 = dt + " | " + cip + ":" + portS + " | " + str;
    logTextEdit->append(str1);
}

void Widget::send(QString str)
{
    if(socketOut->state() != 0)
    {
        if(str.size() != 0)
        {
            Data.clear();
            QDataStream out(&Data, QIODevice::WriteOnly);
            out << qint64(0);
            out << str;
            out.device()->seek(qint64(0));
            out << qint64(Data.size() - sizeof(qint64));
            socketOut->writeDatagram(Data, ipAddr, port);
            qDebug() << "writed to socket!";
            printTE(str);
        }
        else
        {
            QString str2 = "Message cannot be empty";
            printTE(str2);
        }
    }
    else
    {
        QString str1 = "Not connected to server";
        printTE(str1);
    }
}

void Widget::ClickedSlot()
{
    send(messageLineEdit->text());
    messageLineEdit->clear();
}

void Widget::ToggleSlot(bool checked)
{
    if(checked)
    {
        int TEport = portLineEdit->text().toInt();
        if(TEport > 65535)
        {
            QString str1 = "Error, too big number";
            printTE(str1);
            connectButton->toggle();
        }
        else if(TEport < 1)
        {
            QString str1 = "Error, port cannot be negative";
            printTE(str1);
            connectButton->toggle();
        }
        else
        {
            ipAddr = QHostAddress("230.2.3.1").toString();
            port = TEport;
            localAddrs = QNetworkInterface::allAddresses();
            socketIn->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
            socketOut->bind();
            qDebug() << "binded to:";
            qDebug() << ipAddr << port;

            for(const QNetworkInterface &ifcurrent : QNetworkInterface::allInterfaces())
            {
                if(!ifcurrent.flags().testFlag(QNetworkInterface::CanMulticast) || !ifcurrent.flags().testFlag(QNetworkInterface::IsUp))
                    continue;
                QList<QNetworkAddressEntry> entries = ifcurrent.addressEntries();
                for(const QNetworkAddressEntry &entry : entries)
                {
                    if(entry.ip().protocol() == 0)
                    {
                        multicastNetif = ifcurrent;
                        qDebug() << entry.ip();
                        qDebug() << ifcurrent.humanReadableName();
                    }
                }
            }

            socketIn->joinMulticastGroup(ipAddr, multicastNetif);
            socketOut->setMulticastInterface(multicastNetif);

            socketIn->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(1));
            socketOut->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(1));

            QString str = "Connected to multicast group";
            printTE(str);
        }
    }
    else
    {
        if(socketIn->state() != 0)
        {
            socketIn->leaveMulticastGroup(ipAddr, multicastNetif);
            qDebug() << socketIn->state();
            socketIn->close();
            socketOut->close();
            qDebug() << socketIn->state();
            QString str = "Disconnected from multicast group";
            printTE(str);
        }
    }
}
