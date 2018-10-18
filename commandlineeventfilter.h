#ifndef COMMANDLINEEVENTFILTER_H
#define COMMANDLINEEVENTFILTER_H

#include <QObject>

class CommandLineEventFilter : public QObject

{
        Q_OBJECT

    protected:
        bool eventFilter(QObject* obj, QEvent* event);
};

#endif // COMMANDLINEEVENTFILTER_H
