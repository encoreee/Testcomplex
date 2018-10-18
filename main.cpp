#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QStyleFactory>



int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(editabletreemodel);

    QApplication a(argc, argv);
    MainWindow w;

  //  qDebug() << QStyleFactory::keys(); // Доступные схемы

    a.setStyle(QStyleFactory::create("Fusion"));

    w.showMaximized();

    return a.exec();
}
