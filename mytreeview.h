#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QObject>
#include <QTreeView>
#include "test.h"

class MyTreeView : public QTreeView
{
        Q_OBJECT
    public:
        explicit MyTreeView(QWidget *parent = nullptr);

    signals:

    public slots:
};

#endif // MYTREEVIEW_H
