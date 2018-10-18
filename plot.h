#ifndef PLOT_H
#define PLOT_H

#include <QWidget>

namespace Ui {
    class Plot;
}

class Plot : public QWidget
{
        Q_OBJECT

public:

        explicit Plot(QWidget *parent = 0);
        ~Plot();
        Ui::Plot *ui;
private:
private slots:



};

#endif // PLOT_H
