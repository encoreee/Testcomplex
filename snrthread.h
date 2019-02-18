#ifndef SNRTHREAD_H
#define SNRTHREAD_H

#include <QThread>
#include "mainwindow.h"


class snrThread : public QThread
{
     void run() override;
};

#endif // SNRTHREAD_H
