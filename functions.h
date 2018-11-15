#ifndef FUNCTIONS_H
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "interpolation.h"
#include "stdafx.h"
#include <QList>
#include <QDebug>

using namespace alglib;

void makeculculation(const QList<int> &data)
{
   const int SIZE = data.size();
   real_1d_array x,y;
   x.setlength(SIZE);
   y.setlength(SIZE);

   for (int i = 0; i < SIZE; i++)
   {
       x[i] = data.at(i);
       y[i] = i;
   }

//   real_1d_array x = "[0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]";
//  real_1d_array y = "[0.00,0.05,0.26,0.32,0.33,0.43,0.60,0.60,0.77,0.98,1.02]";
   ae_int_t m = 5;
   double t = 2;
   ae_int_t info;
   barycentricinterpolant p;
   polynomialfitreport rep;
   double v;

   //
   // Fitting without individual weights
   //
   // NOTE: result is returned as barycentricinterpolant structure.
   //       if you want to get representation in the power basis,
   //       you can use barycentricbar2pow() function to convert
   //       from barycentric to power representation (see docs for
   //       POLINT subpackage for more info).
   //
   polynomialfit(x, y, m, info, p, rep);
   v = barycentriccalc(p, t);
   printf("%.2f\n", double(v)); // EXPECTED: 2.011

   for (int i = 0; i < SIZE; i++)
   {
     v = barycentriccalc(p, i);
     qDebug() << "Polinom: " << double(v) << endl;
   }


//   //
//   // Fitting with individual weights
//   //
//   // NOTE: slightly different result is returned
//   //
//   real_1d_array w = "[1,1.414213562,1,1,1,1,1,1,1,1,1]";
//   real_1d_array xc = "[]";
//   real_1d_array yc = "[]";
//   integer_1d_array dc = "[]";
//   polynomialfitwc(x, y, w, xc, yc, dc, m, info, p, rep);
//   v = barycentriccalc(p, t);
//   printf("%.2f\n", double(v)); // EXPECTED: 2.023
}
#define FUNCTIONS_H

#endif // FUNCTIONS_H
