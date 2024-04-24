#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Hantek 6000 Oscilloscope DRC converter");
    w.show();
    return a.exec();
}
