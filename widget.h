#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDataStream>
#include <QDateTime>
#include <QHostAddress>
#include <QDoubleValidator>
#include <QRegExpValidator>
#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private:
    QLineEdit *portLineEdit;
    QPushButton *connectButton;
    QTextEdit *logTextEdit;
    QLineEdit *messageLineEdit;
    QPushButton *sendButton;

    QHBoxLayout *topLayout;
    QHBoxLayout *bottomLayout;
    QVBoxLayout *mainLayout;

    QUdpSocket *socketIn;
    QUdpSocket *socketOut;
    QByteArray Data;
    QHostAddress ipAddr;
    QNetworkInterface multicastNetif;
    QList<QHostAddress> localAddrs;
    int port;

    void printTE(QString str);
    void printFromOther(QString str, QString cip, quint16 *cport);
    void send(QString str);

public slots:
    void slotReadyRead();

private slots:
    void ClickedSlot();
    void ToggleSlot(bool checked);
};

#endif // WIDGET_H
