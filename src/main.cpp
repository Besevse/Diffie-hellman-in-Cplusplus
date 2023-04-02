#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication qapp(argc, argv);
    MainWindow w;
    w.show();
    return qapp.exec();
}
