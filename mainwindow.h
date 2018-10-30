#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "test.h"
//#include "commandlineeventfilter.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QList>
#include <QSerialPort>
#include <QDateTime>
#include <QFileInfo>

class SettingsDialog;

class MainWindow : public QMainWindow , private Ui::MainWindow
{
        Q_OBJECT

    signals:

        void haveData(const QByteArray data);
        void getItemData(const QByteArray &data);

    public:

         MainWindow(QWidget *parent = 0);
         ~MainWindow();

    public slots:

        void updateActions();

    private:

        QList <Test> testData;
        QList<int> verticalSizes; // Вертикальный разделитель
        QList<int> horizontalSizes; // Горизонтальный разделитель
        QDateTime nowDateTime = QDateTime::currentDateTime();
        QDate Date;
        QTime Time;
        QString nowDate;
        QString nowTime;
        QString tempString;
        QStringList tempList;
        QFileInfo logFileInfo;
        Test test;
        Test *testPtr = nullptr;
        Test mytest;
        QByteArray reseaveData = "";
        QByteArray buffer = "";
        QString message = "";
        SettingsDialog *m_settings = nullptr;
        QSerialPort *m_serial = nullptr;

//        CommandLineEventFilter* key = new CommandLineEventFilter();

    private slots:

        void cleanLogSpace();
        void insertChild();
        bool insertColumn();
        void insertRow();
        bool removeColumn();
        void removeRow();
        void readLogFile();
//        void testTempFiles();
//        void saveTestToFile();
        void initPortActionsConnections();
        void openSerialPort();
        void closeSerialPort();
        void writeData(const QByteArray &data);
        void readData();
        void handleError(QSerialPort::SerialPortError error);
//        void makeTest();
        void collectdata(const QByteArray &data);
        void printData(const QByteArray &data);
        void cancelSelection();
        bool eventFilter(QObject *object, QEvent *event);
        void Writetestdatatoitem(Test temptest);
        void WriteItemDataToTest(Test *data);
        void BottomWrite();
        void resizeColumn();
        void TestWroteDate();
        void SelectReaction();
        void GetData();


};

#endif // MAINWINDOW_H
