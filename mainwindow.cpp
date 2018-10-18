#include "mainwindow.h"
#include "mytreemodel.h"
#include "Fstring.h"
#include "settingsdialog.h"
#include "commandLine.h"

#include <QtWidgets>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
     m_settings(new SettingsDialog),
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
    actionMaketest1->setEnabled(false);


    QStringList headers;
    headers << tr("Test name") << tr("Date");

    QFile headersfile(":/default.txt");
    headersfile.open(QIODevice::ReadOnly);
    TreeModel *model = new TreeModel(headers);
//    TreeModel *model = new TreeModel(headers, headersfile.readAll());
    headersfile.close();

    workSpace->setModel(model);

    for (int column = 0; column < model->columnCount(); ++column)
        workSpace->resizeColumnToContents(column);

    connect(actionExit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(workSpace->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
    connect(menuFile, &QMenu::aboutToShow, this, &MainWindow::updateActions);
    connect(actionNew_test, &QAction::triggered, this, &MainWindow::insertRow);
    connect(actionDelete_workspace, &QAction::triggered, this, &MainWindow::removeRow);
    connect(actionAdd_existing_file, &QAction::triggered, this, &MainWindow::readLogFile);
    connect(actionRead_log, &QAction::triggered, this, &MainWindow::readLogFile);
    connect(actionClean_logspace, &QAction::triggered, this, &MainWindow::cleanLogSpace);
    connect(actionMake_SNR_test, &QAction::triggered, this, &MainWindow::testTempFiles);
    connect(actionSave_test_as, &QAction::triggered, this, &MainWindow::saveTestToFile);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(commandLine, &CommandLine::getData, this, &MainWindow::writeData);
    connect(pushButton, &QPushButton::clicked, commandLine, &CommandLine::sendBybotton);
    connect(commandLine, &CommandLine::returnPressed, commandLine, &CommandLine::sendBybotton);
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(actionMaketest1,&QAction::triggered, this, &MainWindow::makeTest);
    connect(this, &MainWindow::haveData,this, &MainWindow::printData);

    initPortActionsConnections();

//    connect(actionN, &QAction::triggered, this, &MainWindow::insertChild);
//    connect(removeColumnAction, &QAction::triggered, this, &MainWindow::removeColumn); // Позже допилить
//    connect(insertColumnAction, &QAction::triggered, this, &MainWindow::insertColumn); // Позже допилить

    updateActions();
}

MainWindow::~MainWindow()
{
    delete m_settings;
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

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    workSpace->selectionModel()->setCurrentIndex(model->index(0, 0, index),QItemSelectionModel::ClearAndSelect);

    updateActions();
}

bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = workSpace->model();
    int column = workSpace->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
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

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
    }
}

bool MainWindow::removeColumn()
{
    QAbstractItemModel *model = workSpace->model();
    int column = workSpace->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
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
    bool hasSelection = !workSpace->selectionModel()->selection().isEmpty();

    actionDelete_workspace->setEnabled(hasSelection);

//    removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = workSpace->selectionModel()->currentIndex().isValid();
    actionOpen_workspace->setEnabled(hasCurrent);
//    insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        workSpace->closePersistentEditor(workSpace->selectionModel()->currentIndex());

        int row = workSpace->selectionModel()->currentIndex().row();

//        int column = workSpace->selectionModel()->currentIndex().column();

        if (workSpace->selectionModel()->currentIndex().parent().isValid())
        {
          QMainWindow::statusBar()->showMessage(tr("Position: Log(%1)").arg(row));
        }
        else
        {
          QMainWindow::statusBar()->showMessage(tr("Position: Test(%1) ").arg(row));
        }
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

        test.m_logData.push_back(tempList);
        test.m_directory.push_back(PathName);
        test.m_fileName.push_back(logFileInfo.fileName());
        test.m_readingDateTime.push_back(nowDateTime = QDateTime::currentDateTime());

        QMessageBox::information(this,"Attention","File sucsesfuly read");
        QMainWindow::statusBar()->showMessage(tr("File %1 sucsesfuly read").arg(logFileInfo.fileName()), 5000);
    }
}

void MainWindow::testTempFiles()
{
    qDebug() << test.m_logData.at(0).at(0);
    qDebug() << test.m_directory.at(0);
    qDebug() << test.m_fileName.at(0);
    qDebug() << test.m_readingDateTime.at(0).time().toString("hh:mm:ss");
    qDebug() << test.m_logData.at(1).at(0);
    qDebug() << test.m_directory.at(1);
    qDebug() << test.m_fileName.at(1);
    qDebug() << test.m_readingDateTime.at(1).time().toString("hh:mm:ss");
    qDebug() << test.m_logData.at(2).at(0);
    qDebug() << test.m_directory.at(2);
    qDebug() << test.m_fileName.at(2);
    qDebug() << test.m_readingDateTime.at(2).time().toString("hh:mm:ss");
}

void MainWindow::saveTestToFile()
{
    QString PathName = QFileDialog::getSaveFileName(this, tr("Save WorkSpace"), "", tr("WorkSpace (*.wsp);;All Files (*)"));

    if(!PathName.isEmpty())
    {
        QFile workSpaceFile (PathName);

        workSpaceFile.open(QIODevice::WriteOnly | QIODevice::Text);

            QByteArray tempArray;
            QTextStream tempStream(&workSpaceFile);
            QList<QString>::iterator tempFileNameIterator;
            QList<QString>::iterator tempDirectoryIterator;
            QList<QDateTime>::iterator tempReadindDataTimeIterator;
            QList<QStringList>::iterator tempLogDataIterator;
            tempDirectoryIterator = test.m_directory.begin();
            tempReadindDataTimeIterator = test.m_readingDateTime.begin();
            tempLogDataIterator = test.m_logData.begin();

            for (tempFileNameIterator = test.m_fileName.begin(); tempFileNameIterator != test.m_fileName.end(); ++tempFileNameIterator)
            {
                tempArray = (*tempFileNameIterator).toLocal8Bit();
                workSpaceFile.write(tempArray);
                tempStream << endl;

                tempArray = (*tempDirectoryIterator).toLocal8Bit();
                workSpaceFile.write(tempArray);
                tempStream << endl;

                tempArray = (*tempReadindDataTimeIterator).date().toString("dd.MM.yy").toLocal8Bit();
                workSpaceFile.write(tempArray);
                tempStream << endl;

                tempArray = (*tempReadindDataTimeIterator).time().toString("hh:mm:ss").toLocal8Bit();
                workSpaceFile.write(tempArray);
                tempStream << endl;

                for(int i = 0; i < (*tempLogDataIterator).size(); i++)
                {
                    tempArray = (*tempLogDataIterator).at(i).toLocal8Bit();
                    workSpaceFile.write(tempArray);
                    tempStream << endl;
                }

                tempDirectoryIterator++;
                tempReadindDataTimeIterator++;
                tempLogDataIterator++;

                tempArray.clear();
            }
            workSpaceFile.close();
    }

    else if(PathName.isEmpty())
    {
        qDebug() << "Ошибка записи файла";
        QMessageBox::information(this,"Attention","File saving canceled");
        return;
    }

    QMainWindow::statusBar()->showMessage(tr("File %1 sucsesfuly created").arg(PathName), 5000);
}

void MainWindow::initPortActionsConnections()
{
      connect(actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
      connect(actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
      connect(actionConfigure_serial_port, &QAction::triggered, m_settings, &SettingsDialog::show);     
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
    actionMaketest1->setEnabled(true);


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
    actionMaketest1->setEnabled(false);
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
        qApp->processEvents(0);
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

void MainWindow::makeTest()
{
    QString sral = "SRAL?";
    QString F ="F";
    QByteArray newdata = sral.toLatin1() + '\r';
    writeData(newdata);
    QTime timer;
    timer.start ();
    for(;timer.elapsed() < 1000;)
    {
        qApp->processEvents(0);
    }

    tempString = reseaveData;
    mytest.m_fileName.push_back(tempString);
    mytest.m_readingDateTime.push_back(nowDateTime = QDateTime::currentDateTime());

    QMainWindow::statusBar()->showMessage(tr("The test is performing"));

    logSpace->append("Test started at... ");
    nowDateTime = QDateTime::currentDateTime();
    Date = nowDateTime.date();
    Time = nowDateTime.time();
    nowDate = Date.toString("dd.MM.yy");
    nowTime = Time.toString("hh:mm:ss");
    logSpace->append(nowDate);
    logSpace->append(nowTime);
    logSpace->append("\n");

    QStringList serialList;

    for(int i = 0; i < 50; i++)
        {
            newdata = F.toLatin1() + '\r';
            writeData(newdata);
            QTime timer;
            timer.start () ;
            for(;timer.elapsed() < 300;)
                {
                    qApp->processEvents(0);
                }
            tempString = reseaveData;
            serialList << tempString;
            qDebug() << tempString << "was wrote.";
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

               logSpace->append((*serialListIterator));
               logSpace->append("cleaned");
            }
    mytest.m_logData.push_back(serialList);

    logSpace->append("Test finished at... ");
    nowDateTime = QDateTime::currentDateTime();
    Date = nowDateTime.date();
    Time = nowDateTime.time();
    nowDate = Date.toString("dd.MM.yy");
    nowTime = Time.toString("hh:mm:ss");
    logSpace->append(nowDate);
    logSpace->append(nowTime);

    QMainWindow::statusBar()->showMessage(tr("The test is finished"));
}



