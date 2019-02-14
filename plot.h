#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include "qcustomplot.h"
#include <QInputDialog>

namespace Ui {
    class Plot;
}

enum Styles
{
    STYLE_1,
    STYLE_2,
    STYLE_3,
    STYLE_4,
    STYLE_5
};



class Plot : public QWidget
{
        Q_OBJECT

    public:
        explicit Plot(QWidget *parent = nullptr);
        ~Plot();
        void addGraph(const QList<double> keys, const QList<double> values, Styles style, QString graphName);

    private slots:
      void titleDoubleClick(QMouseEvent *event);
      void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
      void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
      void selectionChanged();
      void mousePress();
      void mouseWheel();
    //  void addRandomGraph();
      void removeSelectedGraph();
      void removeAllGraphs();
      void contextMenuRequest(QPoint pos);
      void moveLegend();
      void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);

    private:
        Ui::Plot *ui;
};

#endif // PLOT_H
