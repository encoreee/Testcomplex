#include "mainwindow.h"
#include "mytreemodel.h"
#include "Fstring.h"
#include "settingsdialog.h"
#include "commandLine.h"
#include "testingdialog.h"
#include "polyfunctions.h"
#include <math.h>
#include "plot.h"

#include <QtWidgets>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QModelIndexList>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
     m_settings(new SettingsDialog),
     m_testSettings (new TestingDialog),
     m_serial(new QSerialPort(this))

{
    setupUi(this);
    this->setWindowIcon(QIcon("myappico.ico"));

    // Размеры разделителей

    verticalSizes << 160 << 700;
    horizontalSizes << 600 << 200 << 25;
    verticalSplitter->setSizes(verticalSizes);
    horizontalSplitter->setSizes(horizontalSizes);
    commandLine->setFixedHeight(25);
    commandLine->setEnabled(false);
    pushButton->setEnabled(false);
    pushButton->setFixedHeight(25);
    pushButton->setFixedWidth(180);
//    commandLine->installEventFilter(key);
    actionConnect->setEnabled(true);
    actionDisconnect->setEnabled(false);
    actionConfigure_serial_port->setEnabled(true);
    actionCollect_data_from_sensor->setEnabled(false);

    QStringList headers;
    headers << tr("Test name") << tr("Date") << tr("Time");

    QFile headersfile(":/default.txt");
    headersfile.open(QIODevice::ReadOnly);
    TreeModel *model = new TreeModel(headers);
//    TreeModel *model = new TreeModel(headers, headersfile.readAll());
    headersfile.close();

    workSpace->setModel(model);

    resizeColumn();

    workSpace->installEventFilter(this);
    logSpace->installEventFilter(this);
    commandLine->installEventFilter(this);

    connect(actionExit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(workSpace->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
 //   connect(menuFile, &QMenu::aboutToShow, this, &MainWindow::updateActions);

    connect(actionAdd_to_test, &QAction::triggered, this, &MainWindow::insertChild);
    connect(actionRemove, &QAction::triggered, this, &MainWindow::removeRow);
    connect(actionAdd_existing_file, &QAction::triggered, this, &MainWindow::readLogFile);
    connect(actionRead_existing_log_file, &QAction::triggered, this, &MainWindow::readLogFile);
    connect(actionClean_logspace, &QAction::triggered, this, &MainWindow::cleanLogSpace);
//    connect(actionMake_SNR_test, &QAction::triggered, this, &MainWindow::testTempFiles);
//    connect(actionSave_test_as, &QAction::triggered, this, &MainWindow::saveTestToFile);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(commandLine, &CommandLine::getData, this, &MainWindow::writeData);
    connect(pushButton, &QPushButton::clicked, commandLine, &CommandLine::sendBybotton);
    connect(commandLine, &CommandLine::returnPressed, commandLine, &CommandLine::sendBybotton);
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(actionCollect_data_from_sensor,&QAction::triggered, this, &MainWindow::collectLogDataFromSensor);
    connect(this, &MainWindow::haveData,this, &MainWindow::printData);
    connect(actionAdd_Test_Data_to_Test_item, &QAction::triggered,this, &MainWindow::bottomWrite);
    connect(model, &TreeModel::itemHaveData, this, &MainWindow::writeItemDataToTest);
    connect(actionTestItem, &QAction::triggered, this, &MainWindow::testWroteDate);
    connect(actionGetData, &QAction::triggered, this, &MainWindow::selectReaction);
    connect(actionMake_test, &QAction::triggered, this, &MainWindow::makeTest);
//   connect(model, &TreeModel::dataChanged, this, &MainWindow::selectReaction);
    //connect(workSpace->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    connect(actionMakeSNRtest, &QAction::triggered, this, &MainWindow::calculateSNR);

    initPortActionsConnections();
    testDialogInitActions();

//    connect(removeColumnAction, &QAction::triggered, this, &MainWindow::removeColumn); // Позже допилить
//    connect(actioninsertColumn, &QAction::triggered, this, &MainWindow::insertColumn); // Позже допилить

    updateActions();
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_testSettings;
}

void MainWindow::initPortActionsConnections()
{
      connect(actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
      connect(actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
      connect(actionConfigure_serial_port, &QAction::triggered, m_settings, &SettingsDialog::show);
}

void MainWindow::testDialogInitActions()
{
      connect(actionNew_test, &QAction::triggered, this, &MainWindow::createTest);
}

void MainWindow::cleanLogSpace()
{
    logSpace->clear();
}

void MainWindow::insertChild()
{
    QModelIndex index = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();

    if (model->columnCount(index) == 0)
    {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column)
    {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    workSpace->selectionModel()->setCurrentIndex(model->index(0, 0, index),QItemSelectionModel::ClearAndSelect);

    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();
    resizeColumn();
    updateActions();
}

bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = workSpace->model();
    int column = workSpace->selectionModel()->currentIndex().column();

    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

    updateActions();

    return changed;
}

void MainWindow::insertRow()
{

    QModelIndex index = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    for (int column = 0; column < model->columnCount(index.parent()); ++column)
    {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
    }

    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();
    resizeColumn();
    updateActions();
}

void MainWindow::insertTest(QString name)
{

    QModelIndex index = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");

        QModelIndex child = model->index(index.row()+1, 0, index.parent());
        model->setData(child, QVariant(name), Qt::DisplayRole);

        child = model->index(index.row()+1, 1, index.parent());
        model->setData(child, QVariant(nowDate), Qt::DisplayRole);

        child = model->index(index.row()+1, 2, index.parent());
        model->setData(child, QVariant(nowTime), Qt::DisplayRole);

    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();
    resizeColumn();
    updateActions();

}

void MainWindow::createTest()
{
    m_testSettings->exec();
    if(m_testSettings->result())
    {
       TestingDialog::TestSettings s = m_testSettings->createTest();

       if(s.choosenTest == TestingDialog::Tests::SNR)
       {
           QString name = "SNR";
           insertTest(name);
       }

       if(s.choosenTest == TestingDialog::Tests::WarmingUp)
       {
           QString name = "WarmingUp";
           insertTest(name);
       }

       if(s.choosenTest == TestingDialog::Tests::Crush)
       {
           QString name = "Crush";
           insertTest(name);
       }

    }
    else
    {
        m_testSettings->close();
    }
}

void MainWindow::makeTest()
{
    QModelIndex selectedIndex = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();

    int row = 0; int colomn = 0;
    while (selectedIndex.child(row,colomn).isValid())
    {
        QModelIndex childindex = selectedIndex.child(row,colomn);

        workSpace->setCurrentIndex(childindex);
        model->data(childindex, Qt::EditRole);
        getData();
      //  testWroteDate();
        calculateSNR();
        row++;
        QTime timer;
        timer.start ();
        for(;timer.elapsed() < 300;)
                    {
                       qApp->processEvents(nullptr);
                    }
        outputTest.cleanData();

    }

//    while (selectedIndex.child(row,colomn).isValid())
//    {
//        QModelIndex childindex = selectedIndex.child(row,colomn);

//        workSpace->setCurrentIndex(childindex);
//        model->data(childindex, Qt::EditRole);
//        QTime timer;
//        timer.start ();
//        getData();
//        for(;timer.elapsed() < 1000;)
//            {
//                qApp->processEvents(nullptr);
//            }
//        calculateSNR();
//        row++;
//    }

    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();


}



bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == workSpace && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
                workSpace->selectionModel()->clearSelection();
                workSpace->selectionModel()->reset();
                bool hasCurrent = workSpace->selectionModel()->currentIndex().isValid();
                updateActions();
                if(hasCurrent)
                {
                    workSpace->selectionModel()->clearSelection();
                    workSpace->selectionModel()->reset();
                    updateActions();
                }
        return true;
        } 
    }
    if (object == logSpace  && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if(testPerforming)
            {
                breakingPoint = true;
                return true;
            }
        }
    }
    if (object == commandLine  && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if(testPerforming)
            {
                breakingPoint = true;
                return true;
            }
        }
    }

    return false;
}

bool MainWindow::removeColumn()
{
    QAbstractItemModel *model = workSpace->model();
    int column = workSpace->selectionModel()->currentIndex().column();

    bool changed = model->removeColumn(column);

    if (changed)
        updateActions();

    return changed;
}

void MainWindow::removeRow()
{
    QModelIndex index = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}

void MainWindow::updateActions()
{
    bool hasCurrent = workSpace->selectionModel()->currentIndex().isValid();

    if (hasCurrent)
    {
        if (!workSpace->selectionModel()->currentIndex().parent().isValid())
        {
        actionAdd_to_test->setEnabled(true);
        actionNew_test->setDisabled(true);
        }

        if (workSpace->selectionModel()->currentIndex().parent().isValid())
        {
        actionAdd_to_test->setDisabled(true);
        actionNew_test->setDisabled(true);
        }

        if(workSpace->selectionModel()->currentIndex().column() != 0)
        {
            workSpace->selectionModel()->clearSelection();
            workSpace->selectionModel()->reset();
            updateActions();
        }

        workSpace->closePersistentEditor(workSpace->selectionModel()->currentIndex());

        int row = workSpace->selectionModel()->currentIndex().row();

        int column = workSpace->selectionModel()->currentIndex().column();

        if (workSpace->selectionModel()->currentIndex().parent().isValid())
        {
          QMainWindow::statusBar()->showMessage(tr("Position: Log(%1).(%2)").arg(row).arg(column));
        }
        else
        {
          QMainWindow::statusBar()->showMessage(tr("Position: Test(%1).(%2)").arg(row).arg(column));
        }
    }
    else
    {
        actionNew_test->setEnabled(true);
        actionAdd_to_test->setDisabled(true);
    }

}

void MainWindow::readLogFile()
{
    QString PathName = QFileDialog::getOpenFileName(this, "Open File", "", "*.txt");
    QFile logFile (PathName);

    if(logFile.isOpen())
    {
        qDebug() << "File was opened";
    }

    logFileInfo.setFile(PathName);

    if(logFile.open(QIODevice::ReadOnly))
    {
        QTextStream tempStream(&logFile);

        while (!tempStream.atEnd())
                {
                   tempString = tempStream.readAll();
                }

        if(tempStream.status()!= QTextStream::Ok)
                {
                    qDebug() << "Ошибка чтения файла";
                    QMessageBox::information(this,"Attention","File reading error.");
                }

        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");

        logSpace->append("File reading started at... ");
        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("\n");

        tempList = tempString.split(QRegExp("\r"), QString::SkipEmptyParts);

        QList<QString>::iterator tempListIterator;

        for (tempListIterator = tempList.begin(); tempListIterator != tempList.end(); ++tempListIterator)
                {
                   int n = (*tempListIterator).size();

                   if (n < 5)
                       {
                           tempList.erase(tempListIterator);
                       }

                   (*tempListIterator).remove("\n");
                   (*tempListIterator).replace("\t"," ");
                   logSpace->append((*tempListIterator));
                }
        logSpace->append("\n");
        logSpace->append("File reading finished at... ");

        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");

        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("\n");

        inputTest.m_logData = tempList;
        inputTest.m_directory = PathName;
        inputTest.m_fileName = logFileInfo.fileName();
        inputTest.m_readingDateTime = nowDateTime = QDateTime::currentDateTime();

        bool select;
        QString testName = QInputDialog::getText(nullptr,"Enter logfile name","Name:", QLineEdit::Normal, "MyLogFile", &select);
        inputTest.setTestName(testName);

        // Если была нажата кнопка Cancel
        if (!select)
        {
            testName = "Log File" + QString::number(++logFileID);
            inputTest.setTestName(testName);
        }

        QMessageBox::information(this,"Attention","File sucsesfuly read");
        QMainWindow::statusBar()->showMessage(tr("File %1 sucsesfuly read").arg(logFileInfo.fileName()), 5000);
    }
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    pushButton->setEnabled(true);
    actionCollect_data_from_sensor->setEnabled(true);


    if (m_serial->open(QIODevice::ReadWrite))
    {
        commandLine->setEnabled(true);
        commandLine->setLocalEchoEnabled(p.localEchoEnabled);
        actionConnect->setEnabled(false);
        actionDisconnect->setEnabled(true);
        actionConfigure_serial_port->setEnabled(false);
        QMainWindow::statusBar()->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                         .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                         .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl), 5000);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        QMainWindow::statusBar()->showMessage(tr("Open error"), 5000);

    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    commandLine->setEnabled(false);
    pushButton->setEnabled(false);
    actionConnect->setEnabled(true);
    actionDisconnect->setEnabled(false);
    actionConfigure_serial_port->setEnabled(true);
    actionCollect_data_from_sensor->setEnabled(false);
    QMainWindow::statusBar()->showMessage(tr("Disconected"), 5000);
}

void MainWindow::writeData(const QByteArray &data)
{
    buffer = "";
    m_serial->write(data);
    if(commandLine->m_localEchoEnabled)
    {
        QString stringdata = data;
        logSpace->append(stringdata);
    } 
}

void MainWindow::readData()
{
    QTime timer;
    timer.start () ;
    for(;timer.elapsed() < 100;)
    {
        qApp->processEvents(nullptr);
    }
    reseaveData = "";
    reseaveData = m_serial->readAll();
    emit haveData(reseaveData);
}

void MainWindow::collectdata(const QByteArray &data)
{
    buffer.append(data);
    qDebug() << buffer;
}

void MainWindow::printData(const QByteArray &data)
{
        QString string = data;
        string.replace("\t"," ");
        string.remove("\n");
        string.remove("\r");
        logSpace->append(string);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::collectLogDataFromSensor()
{
    inputTest.cleanData(); // Очищаем входной контейнер

    QMessageBox::StandardButton question;
    question = QMessageBox::question(this, QString::fromUtf8("Attention!"),
                                     QString::fromUtf8("Do you want to create log file?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (question == QMessageBox::Yes)
    {

        bool select;
        QString testName = QInputDialog::getText(nullptr,
                                                 "Enter logfile name",
                                                 "Name:",
                                                 QLineEdit::Normal,
                                                 "MyLogFile",
                                                 &select);
        inputTest.setTestName(testName);

        if (!select)     // Если была нажата кнопка Cancel
        {
            testName = "Log File" + QString::number(++logFileID);
            inputTest.setTestName(testName);
        }

        QString sral = "SRAL?";
        QString F ="F";
        QByteArray newdata = sral.toLatin1() + '\r';
        writeData(newdata);
        QTime timer;
        timer.start();

        for(;timer.elapsed() < 1000;)
        {
            qApp->processEvents(nullptr);
        }

        tempString = reseaveData;
        QString filename = inputTest.m_fileName = tempString;
        filename.remove("\r");
        nowDateTime = QDateTime::currentDateTime();
        inputTest.m_readingDateTime = nowDateTime;



        QString PathName = QFileDialog::getSaveFileName(this, tr("Save logfile"),
                                                        filename,
                                                        tr("logfile (*.txt);;All Files (*)"));

        QFile file (PathName);
        bool open = file.open(QIODevice::WriteOnly | QIODevice::Text);
        if (!open)
        qDebug() << "file not opened";

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Collecting data started at... ");
        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");
        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("------------------------------------------------------------------");
        logSpace->append("\n");

        QStringList serialList;
        QTextStream stream(&file);
        testPerforming = true;
        for(int i = 0;; i++)
            {
            QMainWindow::statusBar()->showMessage
                    (tr("The test is performing, set cursor on command line and press ESC to STOP."
                        " Cycle %1 is going").arg(i));
                newdata = F.toLatin1() + '\r';
                writeData(newdata);
                QTime timer;
                timer.start () ;
                for(;timer.elapsed() < 300;)
                    {
                        qApp->processEvents(nullptr);
                    }
                tempString = reseaveData;
                serialList << tempString;
                stream << tempString;
                qDebug() << tempString << "was wrote.";
                if(breakingPoint)
                    break;
                qApp->processEvents(nullptr);
            }
            breakingPoint = false;

            file.close();
            if (stream.status() != QTextStream::Ok)
            {
                qDebug() << "Ошибка записи файла";
            }

        QList<QString>::iterator serialListIterator;

        for (serialListIterator = serialList.begin(); serialListIterator != serialList.end(); ++serialListIterator)
                {
                   int n = (*serialListIterator).size();
                   if (n < 5)
                       {
                           serialList.erase(serialListIterator);
                       }
                   (*serialListIterator).remove("\n");
                   (*serialListIterator).remove("\r");
                   (*serialListIterator).replace("\t"," ");

                   logSpace->append(*serialListIterator);
                   logSpace->append("cleaned");
                }
        inputTest.m_logData = serialList;

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Test finished at... ");
        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");
        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("------------------------------------------------------------------");
        QMainWindow::statusBar()->showMessage(tr("The data is setted, logfile is created, %1 cycles was wrote.")
                                              .arg(serialList.size()),5000);
        testPerforming = false;
    }
    else if (question == QMessageBox::No)
    {

        bool select;
        QString testName = QInputDialog::getText(nullptr,
                                                 "Enter logfile name",
                                                 "Name:",
                                                 QLineEdit::Normal,
                                                 "MyLogFile",
                                                 &select);
        inputTest.setTestName(testName);

        if (!select)     // Если была нажата кнопка Cancel
        {
            testName = "Log File" + QString::number(++logFileID);
            inputTest.setTestName(testName);
        }

        QString sral = "SRAL?";
        QString F ="F";
        QByteArray newdata = sral.toLatin1() + '\r';
        writeData(newdata);
        QTime timer;
        timer.start();

        for(;timer.elapsed() < 1000;)
        {
            qApp->processEvents(nullptr);
        }

        tempString = reseaveData;
        QString filename = tempString;
        filename.remove("\r");
        filename.append(".txt");
        inputTest.m_fileName = filename;
        nowDateTime = QDateTime::currentDateTime();
        inputTest.m_readingDateTime = nowDateTime;

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Collecting data started at... ");
        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");
        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("------------------------------------------------------------------");
        logSpace->append("\n");

        QStringList serialList;

        testPerforming = true;
        for(int i = 0;; i++)
            {
            QMainWindow::statusBar()->showMessage
                    (tr("The test is performing, set cursor on command line and press ESC to STOP."
                        " Cycle %1 is going").arg(i));
                newdata = F.toLatin1() + '\r';
                writeData(newdata);
                QTime timer;
                timer.start () ;
                for(;timer.elapsed() < 300;)
                    {
                        qApp->processEvents(nullptr);
                    }
                tempString = reseaveData;
                serialList << tempString;

                qDebug() << tempString << "was wrote.";
                if(breakingPoint)
                    break;
                qApp->processEvents(nullptr);
            }
            breakingPoint = false;

        QList<QString>::iterator serialListIterator;

        for (serialListIterator = serialList.begin(); serialListIterator != serialList.end(); ++serialListIterator)
                {
                   int n = (*serialListIterator).size();
                   if (n < 5)
                       {
                           serialList.erase(serialListIterator);
                       }
                   (*serialListIterator).remove("\n");
                   (*serialListIterator).remove("\r");
                   (*serialListIterator).replace("\t"," ");

                   logSpace->append(*serialListIterator);
                   logSpace->append("cleaned");
                }
        inputTest.m_logData = serialList;
        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Collecting data finished at... ");
        nowDateTime = QDateTime::currentDateTime();
        Date = nowDateTime.date();
        Time = nowDateTime.time();
        nowDate = Date.toString("dd.MM.yy");
        nowTime = Time.toString("hh:mm:ss");
        logSpace->append(nowDate);
        logSpace->append(nowTime);
        logSpace->append("------------------------------------------------------------------");
        QMainWindow::statusBar()->showMessage(tr("The data is setted, logfile is created, %1 cycles was wrote.")
                                              .arg(serialList.size()),5000);
        testPerforming = false;
    }
    else
    {
        QMainWindow::statusBar()->showMessage(tr("Collecting data from sensor is canceled"),5000);
        qDebug() << "Canceled";
    }


}

void MainWindow::writeTestDataToItem(Test temptest)
{
    QModelIndex index = workSpace->selectionModel()->currentIndex();
    QAbstractItemModel *model = workSpace->model();
    QVariant variant = QVariant::fromValue(temptest);
    model->setData(index, variant);
    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();
    resizeColumn();
}

void MainWindow::writeItemDataToTest(Test *data)
{
    testPtr = data;
}

void MainWindow::bottomWrite()
{
    writeTestDataToItem(inputTest);
    resizeColumn();
}

void MainWindow::resizeColumn()
{
    QAbstractItemModel *model = workSpace->model();
    for (int column = 0; column < model->columnCount(); ++column)
    {
        workSpace->resizeColumnToContents(column);
    }
}

void MainWindow::testWroteDate()
{
   logSpace->append(outputTest.m_directory);
   logSpace->append(outputTest.m_fileName);
   logSpace->append(outputTest.m_testName);
   if(!outputTest.m_logData.isEmpty())
   logSpace->append(outputTest.m_logData.first());

}

void MainWindow::selectReaction()
{
   QModelIndex index = workSpace->selectionModel()->currentIndex();
   QAbstractItemModel *model = workSpace->model();
   model->data(index, Qt::EditRole);
   getData();
}

void MainWindow::getData()
{

    outputTest = *(testPtr);
    outputTest.isEmpty = false;

}

void MainWindow::calculateSNR()
{
    QList<double> termo;
    QList<double> ratioS;
    QList<double> signalUs;
    QList<double> signalUref;

    QList<QString>::iterator OTIterator;

    if(!outputTest.isEmpty)
    {
        for (OTIterator = outputTest.m_logData.begin(); OTIterator != outputTest.m_logData.end(); ++OTIterator)
        {
            QString s = (*OTIterator).toLocal8Bit();
            s.remove(1,0);
            QStringList slist = s.split(' ');
            double termoValue = slist.at(0).toLocal8Bit().toDouble();
            double ratioSValue= slist.at(1).toDouble();
            double signalUsValue = slist.at(2).toDouble();
            double signalUrefValue = slist.at(3).toDouble();

            termo.append(termoValue);
            ratioS.append(ratioSValue);
            signalUs.append(signalUsValue);
            signalUref.append(signalUrefValue);
        }

        Polyfuntions polinomUs(signalUs, 5);
        Polyfuntions polinomUref(signalUref, 5);

        QList <double> poliValueUs = polinomUs.getPolinomValues();
        QList <double> poliValueUref = polinomUref.getPolinomValues();

        double minPolyUs = *std::min_element(poliValueUs.begin(), poliValueUs.end());
        double minPolyUref = *std::min_element(poliValueUref.begin(), poliValueUref.end());

        QList <double> normUs;
        QList <double> normUref;

        for(int i = 0; i < signalUs.size(); i++)
        {
            normUs.append((signalUs.at(i)/poliValueUs.at(i)) * minPolyUs);
            normUref.append((signalUref.at(i)/poliValueUref.at(i)) * minPolyUref);
        }
        double sumUs;
        double sumUref;

        for(double d : normUs)
        {
            sumUs += d;
        }

        for(double d : normUref)
        {
            sumUref += d;
        }

        double averageNormUs = sumUs/normUs.size();
        double averageNormUref = sumUref/normUref.size();

        double sigmaNumeratorUs;
        double sigmaNumeratorUref;

        for(double d : normUs)
        {
            sigmaNumeratorUs += qPow((d - averageNormUs),2);
        }

        for(double d : normUref)
        {
            sigmaNumeratorUref += qPow((d - averageNormUref),2);
        }

        double sigmaUs = qSqrt(sigmaNumeratorUs/normUs.size());
        double sigmaUref = qSqrt(sigmaNumeratorUref/normUref.size());

        double snrValueUs = 20 * log10(averageNormUs/sigmaUs);
        double snrValueUref = 20 * log10(averageNormUref/sigmaUref);

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Calculated SNR:");
        logSpace->append(tr("SNR for Us = %1").arg(snrValueUs));
        logSpace->append(tr("SNR for Uref = %1").arg(snrValueUref));
        logSpace->append("------------------------------------------------------------------");



        QList<double> auxiliaryList;
        for(int i = 0; i < signalUs.size(); i ++)
            auxiliaryList.append(i);


            Plot * plot = new Plot() ;

            plot->addGraph(auxiliaryList, signalUs, STYLE_1, "Signal Us");
            plot->addGraph(auxiliaryList, signalUref, STYLE_2, "Signal Uref");
            plot->addGraph(auxiliaryList, poliValueUs, STYLE_3, "polinomiac values Us");
            plot->addGraph(auxiliaryList, poliValueUref, STYLE_4, "polinomiac values Uref");

            plot->show();

        outputTest.cleanData();

    }

    else
    {
        QMessageBox::information(this,"Attention","No data to processing");
    }
}


//void MainWindow::saveTestToFile()
//{
//    QString PathName = QFileDialog::getSaveFileName(this, tr("Save WorkSpace"), "", tr("WorkSpace (*.wsp);;All Files (*)"));

//    if(!PathName.isEmpty())
//    {
//        QFile workSpaceFile (PathName);

//        workSpaceFile.open(QIODevice::WriteOnly | QIODevice::Text);

//            QByteArray tempArray;
//            QTextStream tempStream(&workSpaceFile);
//            QList<QString>::iterator tempFileNameIterator;
//            QList<QString>::iterator tempDirectoryIterator;
//            QList<QDateTime>::iterator tempReadindDataTimeIterator;
//            QList<QStringList>::iterator tempLogDataIterator;
//            tempDirectoryIterator = test.m_directory.begin();
//            tempReadindDataTimeIterator = test.m_readingDateTime.begin();
//            tempLogDataIterator = test.m_logData.begin();

//            for (tempFileNameIterator = test.m_fileName.begin(); tempFileNameIterator != test.m_fileName.end(); ++tempFileNameIterator)
//            {
//                tempArray = (*tempFileNameIterator).toLocal8Bit();
//                workSpaceFile.write(tempArray);
//                tempStream << endl;

//                tempArray = (*tempDirectoryIterator).toLocal8Bit();
//                workSpaceFile.write(tempArray);
//                tempStream << endl;

//                tempArray = (*tempReadindDataTimeIterator).date().toString("dd.MM.yy").toLocal8Bit();
//                workSpaceFile.write(tempArray);
//                tempStream << endl;

//                tempArray = (*tempReadindDataTimeIterator).time().toString("hh:mm:ss").toLocal8Bit();
//                workSpaceFile.write(tempArray);
//                tempStream << endl;

//                for(int i = 0; i < (*tempLogDataIterator).size(); i++)
//                {
//                    tempArray = (*tempLogDataIterator).at(i).toLocal8Bit();
//                    workSpaceFile.write(tempArray);
//                    tempStream << endl;
//                }

//                tempDirectoryIterator++;
//                tempReadindDataTimeIterator++;
//                tempLogDataIterator++;

//                tempArray.clear();
//            }
//            workSpaceFile.close();
//    }

//    else if(PathName.isEmpty())
//    {
//        qDebug() << "Ошибка записи файла";
//        QMessageBox::information(this,"Attention","File saving canceled");
//        return;
//    }

//    QMainWindow::statusBar()->showMessage(tr("File %1 sucsesfuly created").arg(PathName), 5000);
//}


