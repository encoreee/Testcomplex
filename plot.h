#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include "qcustomplot.h"
#include <QInputDialog>

namespace Ui {
    class Plot;
}

class Plot : public QWidget
{
        Q_OBJECT

    public:
        explicit Plot(QWidget *parent = nullptr);
        ~Plot();

    private slots:
      void titleDoubleClick(QMouseEvent *event);
      void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
      void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
      void selectionChanged();
      void mousePress();
      void mouseWheel();
      void addRandomGraph();
      void removeSelectedGraph();
      void removeAllGraphs();
      void contextMenuRequest(QPoint pos);
      void moveLegend();
      void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

    private:
        Ui::Plot *ui;
};

#endif // PLOT_H
