#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("Multicast chat");
    w.resize(600, 300);
    w.show();

    return a.exec();
}
