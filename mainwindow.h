#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "test.h"
#include "testingdialog.h"
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

         MainWindow(QWidget *parent = nullptr);
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
        Test inputTest;
        Test *testPtr = nullptr;
        Test outputTest;
        QByteArray reseaveData = "";
        QByteArray buffer = "";
        QString message = "";
        SettingsDialog *m_settings = nullptr;
        TestingDialog *m_testSettings = nullptr;
        QSerialPort *m_serial = nullptr;
        unsigned int logFileID = 0;
        bool breakingPoint = false;
        bool testPerforming = false;

//        CommandLineEventFilter* key = new CommandLineEventFilter();

    private slots:

        void cleanLogSpace();
        void insertChild();
        bool insertColumn();
        void insertRow();
        void insertTest(QString name);
        bool removeColumn();
        void removeRow();
        void readLogFile();

        void initPortActionsConnections();
        void testDialogInitActions();

        void openSerialPort();
        void closeSerialPort();
        void writeData(const QByteArray &data);
        void readData();
        void handleError(QSerialPort::SerialPortError error);
        void collectLogDataFromSensor();
        void collectdata(const QByteArray &data);
        void printData(const QByteArray &data);
        bool eventFilter(QObject *object, QEvent *event);
        void writeTestDataToItem(Test temptest);
        void writeItemDataToTest(Test *data);
        void bottomWrite();
        void resizeColumn();
        void testWroteDate();
        void selectReaction();
        void getData();
        void createTest();
        void calculateSNR();
//        void saveTestToFile();

};

#endif // MAINWINDOW_H
