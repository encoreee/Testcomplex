#ifndef CUSTOMTABLE_H
#define CUSTOMTABLE_H

#include <QTableWidget>
#include <QWidget>

class customTable : public QTableWidget
{
         Q_OBJECT

    public:
        explicit customTable(QWidget *parent = nullptr);


};

#endif // CUSTOMTABLE_H
