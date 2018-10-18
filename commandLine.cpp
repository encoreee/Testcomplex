


#include <QKeyEvent>
#include "commandLine.h"

CommandLine::CommandLine(QWidget *parent) :
    QLineEdit(parent)
{

}

void CommandLine::setLocalEchoEnabled(bool set)
{
    m_localEchoEnabled = set;
}

//void CommandLine::keyPressEvent(QKeyEvent *e)
//{
//    switch (e->key())
//    {
//    case Qt::Key_Enter:
//        {
//        QString temp = this->text();
//        QByteArray message = temp.toLatin1() + '\r';
//        emit getData(message);
//        this->clear();
//        }
//        break;
//    default:
//        {
//            QLineEdit::keyPressEvent(e);
//            break;
//        }
//    }
//}

void CommandLine::sendBybotton()
{
    QString temp = this->text();
    QByteArray message = temp.toLatin1() + '\r';
    emit getData(message);
    this->clear();
}

void CommandLine::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}

void CommandLine::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void CommandLine::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}
