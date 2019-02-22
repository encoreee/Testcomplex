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
#include <QPair>

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
    splitter_2->setSizes(verticalSizes);
    splitter->setSizes(horizontalSizes);
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

  // no need
  //  QObject *obj = new QTableWidget;
  //  this->tab_1 = qobject_cast<QTableWidget *>(obj);
  // no need

    connect(actionExit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(workSpace->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
 //   connect(menuFile, &QMenu::aboutToShow, this, &MainWindow::updateActions);

    connect(actionAdd_to_test, &QAction::triggered, this, &MainWindow::insertChild);
    connect(actionRemove, &QAction::triggered, this, &MainWindow::removeRow);
    connect(actionAdd_existing_file, &QAction::triggered, this, &MainWindow::readLogFile);
    connect(actionRead_existing_log_file, &QAction::triggered, this, &MainWindow::readLogFiles);
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
    connect(actionCollect_data_from_device, &QAction::triggered, this, &MainWindow::collectLogDataFromDevice);

    initPortActionsConnections();
    testDialogInitActions();

//    connect(removeColumnAction, &QAction::triggered, this, &MainWindow::removeColumn); // Позже допилить
//    connect(actioninsertColumn, &QAction::triggered, this, &MainWindow::insertColumn); // Позже допилить

    QShortcut *scCtrlC = new QShortcut(QKeySequence("Ctrl+C"), tab_1);
    connect(scCtrlC, SIGNAL(activated()), this, SLOT(copyCell()));
    QShortcut *scCtrlV = new QShortcut(QKeySequence("Ctrl+V"), tab_1);
    connect(scCtrlV, SIGNAL(activated()), this, SLOT(pasteCell()));

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

void MainWindow::copyCell()
{
        QAbstractItemModel* model = tab_1->model();
        QItemSelectionModel* selection = tab_1->selectionModel();
        QModelIndexList indexes = selection->selectedIndexes();

        if (indexes.count() == 0) {
                return;
        }

        qSort(indexes);

        QString clip;
        QModelIndex previous = indexes.first();
        indexes.removeFirst();
        clip.append(model->data(previous).toString());

        for (QModelIndex current : indexes.toVector())
        {
                if (current.row() != previous.row())
                {
                        clip.append("\n");
                }
                else
                {
                        clip.append("\t");
                }

                clip.append(model->data(current).toString());
                previous = current;
        }

        QApplication::clipboard()->setText(clip);
}

void MainWindow::pasteCell()
{
        if (tab_1->selectedItems().size() != 1)
        {
                return;
        }
        QString clip = QApplication::clipboard()->text();
        QStringList rowList = clip.split("\n");

        QTableWidgetItem *currentItem = tab_1->selectedItems().first();
        int pasteRow = currentItem->row();
        int pasteCol = currentItem->column();
        for(QString row : rowList.toVector())
        {
                QStringList colList = row.split("\t");
                for (QString cell : colList.toVector())
                {
                        tab_1->item(pasteRow, pasteCol)->setText(cell);
                        pasteCol++;
                }
                pasteCol = currentItem->column();
                pasteRow++;
        }
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
    QVector<double> snrsUs;
    QVector<double> snrsUref;

    int row = 0; int colomn = 0;

    while (selectedIndex.child(row,colomn).isValid())
    {
        QModelIndex childindex = selectedIndex.child(row,colomn);

        workSpace->setCurrentIndex(childindex);
        model->data(childindex, Qt::EditRole);
        getData();
      //  testWroteDate(); // Debug, dont forget to remove
        QPair<double,double> snrs = calculateSNR();

        if (snrs.first != NULL && snrs.second != NULL)
        {
            snrsUs.append(snrs.first);
            snrsUref.append(snrs.second);
        }

        row++;
        QTime timer;
        timer.start ();
        for(;timer.elapsed() < 300;)
                    {
                       qApp->processEvents(nullptr);
                    }
        outputTest.cleanData();

    }

    QVector<double> keys;

    for (int i = 1; i < snrsUs.size()+1; i++)
    {
        keys.append(static_cast<double>(i));
    }

    double minSnrsUs = *std::min_element(snrsUs.begin(), snrsUs.end());
    double minSnrsUref = *std::min_element(snrsUref.begin(), snrsUref.end());

    double maxSnrsUs = *std::max_element(snrsUs.begin(), snrsUs.end());
    double maxSnrsUref = *std::max_element(snrsUref.begin(), snrsUref.end());
    QPen graphPen;

    tab_2->addGraph();
    tab_2->graph()->setData(keys,snrsUs);
    graphPen.setColor(QColor(255, 0, 0, 127));
    graphPen.setWidthF(2);
    tab_2->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssCircle));
    tab_2->graph()->setPen(graphPen);
    tab_2->replot();

    tab_2->addGraph();
    tab_2->graph()->setData(keys,snrsUref);
    graphPen.setColor(QColor(0, 0, 255, 127));
    graphPen.setWidthF(2);
    tab_2->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssCircle));
    tab_2->graph()->setPen(graphPen);
    tab_2->replot();

    tab_2->xAxis->setLabel("Sensors");
    tab_2->yAxis->setLabel("Signal to noise");

    tab_2->xAxis->setRange(0, snrsUs.size()+1);

    tab_2->yAxis->setRange((minSnrsUs < minSnrsUref ? minSnrsUs : minSnrsUref) - 1 , (maxSnrsUs > maxSnrsUref ? maxSnrsUs : maxSnrsUref) + 1);

    tab_2->replot();

    tab_1->setRowCount(snrsUs.size()+5);
    tab_1->setColumnCount(5);

    tab_1->setItem(0,0,new QTableWidgetItem(QString("SNR in Us canal")));
    tab_1->setItem(0,1,new QTableWidgetItem(QString("SNR in Us canal")));

    for (int i = 0, j = 1; i < snrsUs.size(); i++, j++)
    {
        tab_1->setItem(j,0,new QTableWidgetItem(QString("%1").arg(snrsUs[i])));
        tab_1->setItem(j,1,new QTableWidgetItem(QString("%1").arg(snrsUref[i])));
    }

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

// dont work
//    if (object == tab_1 && event->type() == QEvent::KeyPress)
//    {
//         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//         if (keyEvent->key() == QKeySequence::Copy)
//         {
//             copyCell();
//         }
//    }
// dont work

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

void MainWindow::readLogFiles()
{

    QStringList files = QFileDialog::getOpenFileNames(this,
                                QString::fromUtf8("Выберите один, или более файлов"),
                                QDir::currentPath(),
                                "Images (*.txt);;All files (*.*)");
    for(QString path : files)
    {
        QFile logFile (path);

        if(logFile.isOpen())
        {
            qDebug() << "File was opened";
        }

        logFileInfo.setFile(path);

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
            inputTest.m_directory = path;
            inputTest.m_fileName = logFileInfo.fileName();
            inputTest.m_testName = logFileInfo.fileName();
            inputTest.m_readingDateTime = nowDateTime;

            QModelIndex index = workSpace->selectionModel()->currentIndex();
            QAbstractItemModel *model = workSpace->model();

            logSpace->append("------------------Input test data--------------------------------------");
            logSpace->append(inputTest.m_directory);
            logSpace->append(inputTest.m_fileName);
            logSpace->append(inputTest.m_testName);
            if(!inputTest.m_logData.isEmpty())
            logSpace->append(inputTest.m_logData.first());
              logSpace->append("------------------Input test data--------------------------------------");



            if (model->columnCount(index) == 0)
            {
                if (!model->insertColumn(0, index))
                    return;
            }

            if (!model->insertRow(0, index))
                return;


                QModelIndex child1 = model->index(0, 0, index);
                model->setData(child1, QVariant(inputTest.m_fileName), Qt::EditRole);

                QVariant variant = QVariant::fromValue(inputTest);
                model->setData(child1, variant);

                QModelIndex child2 = model->index(0, 1, index);
                model->setData(child2, QVariant(nowDate), Qt::EditRole);

                QModelIndex child3 = model->index(0, 2, index);
                model->setData(child3, QVariant(nowTime), Qt::EditRole);

                getData();

                logSpace->append("------------------Output test data--------------------------------------");
                logSpace->append(outputTest.m_directory);
                logSpace->append(outputTest.m_fileName);
                logSpace->append(outputTest.m_testName);
                if(!outputTest.m_logData.isEmpty())
                logSpace->append(outputTest.m_logData.first());
                logSpace->append("------------------Output test data--------------------------------------");






//            QVariant variant = QVariant::fromValue(temptest);
//            model->setData(index, variant);
//            workSpace->selectionModel()->clearSelection();
//            workSpace->selectionModel()->reset();
//            resizeColumn();


//            QVariant variant = QVariant::fromValue(temptest);
//            model->setData(index, variant);

//            resizeColumn();
//            updateActions();


//            bool select;
//            QString testName = QInputDialog::getText(nullptr,"Enter logfile name","Name:", QLineEdit::Normal, "MyLogFile", &select);
//            inputTest.setTestName(testName);

//            // Если была нажата кнопка Cancel
//            if (!select)
//            {
//                testName = "Log File" + QString::number(++logFileID);
//                inputTest.setTestName(testName);
//            }

//          //  QMessageBox::information(this,"Attention","File sucsesfuly read");


            QMainWindow::statusBar()->showMessage(tr("File %1 sucsesfuly read").arg(logFileInfo.fileName()), 5000);


        }
    }

    QModelIndex index = workSpace->selectionModel()->currentIndex();
    workSpace->setExpanded(index, true);
    resizeColumn();
    updateActions();
    workSpace->selectionModel()->clearSelection();
    workSpace->selectionModel()->reset();




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

void MainWindow::collectLogDataFromDevice()
{
    inputTest.cleanData(); // Очищаем входной контейнер

    if (m_serial->isOpen())
    {

        QString sral = "SRAL?";
        QString f ="F";
        QString rev = "REV?";
        QString device;
        QString checkedSensors;
        QString initSensors;

        QByteArray newdata = rev.toLatin1() + '\r';
        writeData(newdata);
        QTime timer;
        timer.start();

        for(;timer.elapsed() < 500;)
        {
            qApp->processEvents(nullptr);
        }

        tempString = reseaveData;
        device = tempString;


        for(int i = 0x1; i < 0x0C; i++)
        {
            tempString = "";

            if (i < 0x10 )
            {
                initSensors = QString::fromUtf8("#0%1").arg(i);
            }

            else
            {
                initSensors = QString::fromUtf8("#%1").arg(i);
            }

            newdata = initSensors.toLatin1() + sral.toLatin1() +'\r';
            writeData(newdata);
            timer.restart();
            for(;timer.elapsed() < 1500;)
            {
                qApp->processEvents(nullptr);
            }

            tempString = reseaveData;

            if(!tempString.isEmpty() && !(tempString == "FAL"))
            {
                checkedSensors += "Sensor " + initSensors + " is ok\n";
            }
            else if (tempString.isEmpty() || tempString == "FAL")
            {
                checkedSensors += "Sensor " + initSensors + " is not connected\n";
            }

        }

        QMessageBox::StandardButton question = QMessageBox::question(this, QString::fromUtf8("Attention!"),
                                               QString("Device %1 was connected. \n" + checkedSensors).arg(device),
                                               QMessageBox::Apply | QMessageBox::Retry | QMessageBox::Cancel);


//        bool select;
//        QString testName = QInputDialog::getText(nullptr,
//                                                 "Enter logfile name",
//                                                 "Name:",
//                                                 QLineEdit::Normal,
//                                                 "MyLogFile",
//                                                 &select);
//        inputTest.setTestName(testName);

//        if (!select)     // Если была нажата кнопка Cancel
//        {
//            testName = "Log File" + QString::number(++logFileID);
//            inputTest.setTestName(testName);
//        }



//        writeData(newdata);
//        QTime timer;
//        timer.start();

//        for(;timer.elapsed() < 500;)
//        {
//            qApp->processEvents(nullptr);
//        }


//        QString filename = inputTest.m_fileName = tempString;
//        filename.remove("\r");
//        nowDateTime = QDateTime::currentDateTime();
//        inputTest.m_readingDateTime = nowDateTime;



//        QString PathName = QFileDialog::getSaveFileName(this, tr("Save logfile"),
//                                                        filename,
//                                                        tr("logfile (*.txt);;All Files (*)"));

//        QFile file (PathName);
//        bool open = file.open(QIODevice::WriteOnly | QIODevice::Text);
//        if (!open)
//        qDebug() << "file not opened";

//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("Collecting data started at... ");
//        nowDateTime = QDateTime::currentDateTime();
//        Date = nowDateTime.date();
//        Time = nowDateTime.time();
//        nowDate = Date.toString("dd.MM.yy");
//        nowTime = Time.toString("hh:mm:ss");
//        logSpace->append(nowDate);
//        logSpace->append(nowTime);
//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("\n");

//        QStringList serialList;
//        QTextStream stream(&file);
//        testPerforming = true;
//        for(int i = 0;; i++)
//            {
//            QMainWindow::statusBar()->showMessage
//                    (tr("The test is performing, set cursor on command line and press ESC to STOP."
//                        " Cycle %1 is going").arg(i));
//                newdata = F.toLatin1() + '\r';
//                writeData(newdata);
//                QTime timer;
//                timer.start () ;
//                for(;timer.elapsed() < 300;)
//                    {
//                        qApp->processEvents(nullptr);
//                    }
//                tempString = reseaveData;
//                serialList << tempString;
//                stream << tempString;
//                qDebug() << tempString << "was wrote.";
//                if(breakingPoint)
//                    break;
//                qApp->processEvents(nullptr);
//            }
//            breakingPoint = false;

//            file.close();
//            if (stream.status() != QTextStream::Ok)
//            {
//                qDebug() << "Ошибка записи файла";
//            }

//        QList<QString>::iterator serialListIterator;

//        for (serialListIterator = serialList.begin(); serialListIterator != serialList.end(); ++serialListIterator)
//                {
//                   int n = (*serialListIterator).size();
//                   if (n < 5)
//                       {
//                           serialList.erase(serialListIterator);
//                       }
//                   (*serialListIterator).remove("\n");
//                   (*serialListIterator).remove("\r");
//                   (*serialListIterator).replace("\t"," ");

//                   logSpace->append(*serialListIterator);
//                   logSpace->append("cleaned");
//                }
//        inputTest.m_logData = serialList;

//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("Test finished at... ");
//        nowDateTime = QDateTime::currentDateTime();
//        Date = nowDateTime.date();
//        Time = nowDateTime.time();
//        nowDate = Date.toString("dd.MM.yy");
//        nowTime = Time.toString("hh:mm:ss");
//        logSpace->append(nowDate);
//        logSpace->append(nowTime);
//        logSpace->append("------------------------------------------------------------------");
//        QMainWindow::statusBar()->showMessage(tr("The data is setted, logfile is created, %1 cycles was wrote.")
//                                              .arg(serialList.size()),5000);
//        testPerforming = false;
//    }
//    else if (question == QMessageBox::No)
//    {

//        bool select;
//        QString testName = QInputDialog::getText(nullptr,
//                                                 "Enter logfile name",
//                                                 "Name:",
//                                                 QLineEdit::Normal,
//                                                 "MyLogFile",
//                                                 &select);
//        inputTest.setTestName(testName);

//        if (!select)     // Если была нажата кнопка Cancel
//        {
//            testName = "Log File" + QString::number(++logFileID);
//            inputTest.setTestName(testName);
//        }

//        QString sral = "SRAL?";
//        QString F ="F";
//        QByteArray newdata = sral.toLatin1() + '\r';
//        writeData(newdata);
//        QTime timer;
//        timer.start();

//        for(;timer.elapsed() < 1000;)
//        {
//            qApp->processEvents(nullptr);
//        }

//        tempString = reseaveData;
//        QString filename = tempString;
//        filename.remove("\r");
//        filename.append(".txt");
//        inputTest.m_fileName = filename;
//        nowDateTime = QDateTime::currentDateTime();
//        inputTest.m_readingDateTime = nowDateTime;

//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("Collecting data started at... ");
//        nowDateTime = QDateTime::currentDateTime();
//        Date = nowDateTime.date();
//        Time = nowDateTime.time();
//        nowDate = Date.toString("dd.MM.yy");
//        nowTime = Time.toString("hh:mm:ss");
//        logSpace->append(nowDate);
//        logSpace->append(nowTime);
//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("\n");

//        QStringList serialList;

//        testPerforming = true;
//        for(int i = 0;; i++)
//            {
//            QMainWindow::statusBar()->showMessage
//                    (tr("The test is performing, set cursor on command line and press ESC to STOP."
//                        " Cycle %1 is going").arg(i));
//                newdata = F.toLatin1() + '\r';
//                writeData(newdata);
//                QTime timer;
//                timer.start () ;
//                for(;timer.elapsed() < 300;)
//                    {
//                        qApp->processEvents(nullptr);
//                    }
//                tempString = reseaveData;
//                serialList << tempString;

//                qDebug() << tempString << "was wrote.";
//                if(breakingPoint)
//                    break;
//                qApp->processEvents(nullptr);
//            }
//            breakingPoint = false;

//        QList<QString>::iterator serialListIterator;

//        for (serialListIterator = serialList.begin(); serialListIterator != serialList.end(); ++serialListIterator)
//                {
//                   int n = (*serialListIterator).size();
//                   if (n < 5)
//                       {
//                           serialList.erase(serialListIterator);
//                       }
//                   (*serialListIterator).remove("\n");
//                   (*serialListIterator).remove("\r");
//                   (*serialListIterator).replace("\t"," ");

//                   logSpace->append(*serialListIterator);
//                   logSpace->append("cleaned");
//                }
//        inputTest.m_logData = serialList;
//        logSpace->append("------------------------------------------------------------------");
//        logSpace->append("Collecting data finished at... ");
//        nowDateTime = QDateTime::currentDateTime();
//        Date = nowDateTime.date();
//        Time = nowDateTime.time();
//        nowDate = Date.toString("dd.MM.yy");
//        nowTime = Time.toString("hh:mm:ss");
//        logSpace->append(nowDate);
//        logSpace->append(nowTime);
//        logSpace->append("------------------------------------------------------------------");
//        QMainWindow::statusBar()->showMessage(tr("The data is setted, logfile is created, %1 cycles was wrote.")
//                                              .arg(serialList.size()),5000);
//        testPerforming = false;
//    }
//    else
//    {
//        QMainWindow::statusBar()->showMessage(tr("Serial port is not initialized"),5000);
//        qDebug() << "Canceled";
//    }

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

QPair<double,double> MainWindow::calculateSNR()
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

        QList<double> coefUs = polinomUs.getCoefficients();
        QList<double> coefUref = polinomUref.getCoefficients();

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Debag data:");

        for(double d : coefUs)
        {
            logSpace->append(tr("Coefficient Us: %1").arg(d));
        }

        logSpace->append("");

        for(double d : coefUref)
        {
            logSpace->append(tr("Coefficient Uref: %1").arg(d));
        }

        logSpace->append("");


        QList <double> poliValueUs = polinomUs.getPolinomValues();
        QList <double> poliValueUref = polinomUref.getPolinomValues();

        double minPolyUs = *std::min_element(poliValueUs.begin(), poliValueUs.end());
        double minPolyUref = *std::min_element(poliValueUref.begin(), poliValueUref.end());

        logSpace->append(tr("min poly Us: %1").arg(minPolyUs));
        logSpace->append(tr("min poly Uref: %1").arg(minPolyUref));
        logSpace->append("");


        QList <double> normUs;
        QList <double> normUref;

        for(int i = 0; i < signalUs.size(); i++)
        {
            normUs.append((signalUs.at(i)/poliValueUs.at(i)) * minPolyUs);
            normUref.append((signalUref.at(i)/poliValueUref.at(i)) * minPolyUref);
        }

        double sumUs = 0;
        double sumUref = 0;

        for(double normValuesUs : normUs)
        {
            sumUs += normValuesUs;
        }

        for (int i = 0; i < normUref.size(); i++){
            sumUref = sumUref + normUref.at(i);
        }



        logSpace->append(tr("sumUs: %1").arg(sumUs));
        logSpace->append(tr("sumUref: %1").arg(sumUref));
        logSpace->append("");


        logSpace->append(tr("normUs.size: %1").arg(normUs.size()));
        logSpace->append(tr("normUref.size: %1").arg(normUref.size()));
        logSpace->append("");


        double averageNormUs = sumUs/normUs.size();
        double averageNormUref = sumUref/normUref.size();

        logSpace->append(tr("avarage norm us: %1").arg(averageNormUs));
        logSpace->append(tr("avarage norm uref: %1").arg(averageNormUref));
        logSpace->append("");


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

        logSpace->append(tr("Sigma Us: %1").arg(sigmaUs));
        logSpace->append(tr("sigmaUref: %1").arg(sigmaUref));
        logSpace->append("");


        double snrValueUs = 20 * log10(averageNormUs/sigmaUs);
        double snrValueUref = 20 * log10(averageNormUref/sigmaUref);

        logSpace->append("------------------------------------------------------------------");
        logSpace->append("Calculated SNR:");
        logSpace->append(tr("SNR for Us = %1").arg(snrValueUs));
        logSpace->append(tr("SNR for Uref = %1").arg(snrValueUref));
        logSpace->append("------------------------------------------------------------------");
        logSpace->append("");


        QList<double> auxiliaryList;
        for(int i = 0; i < signalUs.size(); i ++)
            auxiliaryList.append(i);


            Plot * plot = new Plot() ;

            plot->addGraph(auxiliaryList, signalUs, STYLE_1, "Signal Us");
            plot->addGraph(auxiliaryList, signalUref, STYLE_2, "Signal Uref");
            plot->addGraph(auxiliaryList, poliValueUs, STYLE_3, "polinomiac values Us");
            plot->addGraph(auxiliaryList, poliValueUref, STYLE_4, "polinomiac values Uref");
            plot->addGraph(auxiliaryList, normUs, STYLE_5, "norm Us");
            plot->addGraph(auxiliaryList, normUref, STYLE_5, "norm Uref");

            plot->show();

        outputTest.cleanData();
        QPair<double,double> snrPair;
        snrPair.first = snrValueUs;
        snrPair.second = snrValueUref;

        return snrPair;
    }

    else
    {
        QMessageBox::information(this,"Attention","No data to processing");
        QPair<double,double> snrPairFault;
        return snrPairFault;
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


