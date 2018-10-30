#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>
#include "test.h"

typedef QPair<QVariant,Test> DataPair;

class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);

    ~TreeItem();

    TreeItem *child(int number);

    int childCount() const;

    int columnCount() const;

    QVariant data(int column) const;

    bool insertChildren(int position, int count, int columns);

    bool insertColumns(int position, int columns);

    TreeItem *parent();

    bool removeChildren(int position, int count);

    bool removeColumns(int position, int columns);

    int childNumber() const;

    bool setData(int column, const QVariant &value);

    void WroteTestData();

    bool getDataFlag();

    void resetDataFlag();

private:


    QList<TreeItem*> childItems;

    QVector<QVariant> itemData;

    TreeItem *parentItem;

    Test test;

    bool dataFlag = false;

};

Q_DECLARE_METATYPE(DataPair)
#endif // TREEITEM_H
