#include "point.h"
#include <QDebug>

Point::Point()

{
    x=0;
    y=0;
    z=0;

}

void Point::setpoint(int mx, int my, int mz)
{
  x=mx;
  y=my;
  z=mz;
}

void Point::showpoint()

{
    qDebug() << x << "-X";
     qDebug() << y <<"-Y";
     qDebug() << z << "-Z";

}
