#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QLineEdit>

class CommandLine : public QLineEdit
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit CommandLine (QWidget *parent = nullptr);
    void setLocalEchoEnabled(bool set);
    void sendBybotton();
    bool m_localEchoEnabled = false;

protected:
  //  void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;


};

#endif // COMMANDLINE_H
