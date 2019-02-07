#ifndef POLYFUNCTIONS_H
#define POLYFUNCTIONS_H

#include <stdio.h>
#include <process.h>
#include <math.h>
#include <QList>

class Polyfuntions
{
    public:

        Polyfuntions();

        ~Polyfuntions();

        void setInputData(QList<double> inputData);
        void setPolyPower(int K);

    private:

        void allocmatrix();
        void freematrix();
        void readmatrix();
        void printresult();
        void diagonal();
        void cls();

        // K<=N only
        int K; //K - polinom power
        int N; //N - number of data points

        QList<double> data;

        double *a;
        double *b;
        double *x;
        double *y;
        double **sums;

};
#endif // POLYFUNCTIONS_H
