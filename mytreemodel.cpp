#include <QtWidgets>
#include "mytreeitem.h"
#include "mytreemodel.h"
#include <QDateTime>


TreeModel::TreeModel(const QStringList &headers, QObject *parent) : QAbstractItemModel (parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem(rootData);
//    setupModelData(data.split(QString("\n")), rootItem);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
       return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);
    QVariant varaint = item->data(index.column());
    DataPair pair = varaint.value<DataPair>();

    emit ItemHaveData(item->testptr);

   return pair.first;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if(value.canConvert<QString>())
    {
        TreeItem *item = getItem(index);
        bool result = item->setData(index.column(), value);

        if (result)
            emit dataChanged(index, index);

        return result;
    }
    else
    {
        TreeItem *item = getItem(index);
        bool result = item->setData(index.column(), value);
        Test test = value.value<Test>();
        QString Date = test.m_readingDateTime.date().toString();
        QString Time = test.m_readingDateTime.time().toString();

        item->setData(index.column(), test.m_testName);
        item->setData(1, Date);
        item->setData(2, Time);
        item->WroteTestData();

        return result;
    }
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    TreeItem *item = getItem(index);
    bool flag = item->getDataFlag();

    if(flag)
    return Qt::ItemIsSelectable | QAbstractItemModel::flags(index);

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QVariant headerdata = rootItem->data(section);
        DataPair pair = headerdata.value<DataPair>();
        return pair.first;
    }
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{

    if(columnCount(parent) < 3)
    {
        bool success;
        beginInsertColumns(parent, position, position + columns - 1);
        success = rootItem->insertColumns(position, columns);
        endInsertColumns();

        return success;
    }
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count())
    {
        int position = 0;
        while (position < lines[number].length())
        {
            if (lines[number].at(position) != ' ')
                break;
            ++position;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty())
        {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QVector<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last())
            {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0)
                {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            }

            else
            {
                while (position < indentations.last() && parents.count() > 0)
                {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            TreeItem *parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column)
                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
        }
        ++number;
    }
}

int TreeModel::childCount(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return -1;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return -1;

    TreeItem *item = getItem(index);

    return item->childCount();
}
