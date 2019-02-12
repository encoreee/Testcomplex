#include <stdio.h>
#include <process.h>
#include <math.h>
#include <QDebug>
#include <iostream>
#include "polyfunctions.h"
#include <iomanip>
#include <QtMath>

using namespace std;

Polyfuntions::Polyfuntions(QList<double> inputData, int K)
{
    setInputData(inputData);
    setPolyPower(K);
    calculate();
}

Polyfuntions::~Polyfuntions()
{
    freematrix();
}

void Polyfuntions::setInputData(QList<double> inputData)
{
    N = inputData.size();

    for (int i = 0; i < inputData.size(); i++) {
        data.append(inputData.at(i));
    }
}

void Polyfuntions::setPolyPower(int K)
{
    this->K = K;
}

void Polyfuntions::allocmatrix()
{
   //allocate memory for matrixes

   a = new double[K+1];
   b = new double[K+1];
   x = new double[N];
   y = new double[N];

   sums = new double*[K+1];


   if(x==nullptr || y==nullptr || a==nullptr || sums==nullptr)
   {
       QString errorString = QString("\nNot enough memory to allocate. N=%1, K=%2\n").arg(N).arg(K);
       qDebug() << errorString;
   }


   for(int i = 0; i < K + 1; i++)
   {
       sums[i] = new double[K+1];
       if(sums[i] == nullptr)
       {
           QString errorString = QString("\nNot enough memory to allocate for %d equations.\n").arg(K+1);
           qDebug() << errorString;
       }
   }

   for(int i = 0; i < K + 1; i++)
   {
       a[i] = 0;
       b[i] = 0;
       for(int j = 0; j<K+1; j++)
       {
           sums[i][j] = 0;
       }
   }

   for(int k = 0; k<N; k++)
   {
       x[k] = 0;
       y[k] = 0;
   }
}

void Polyfuntions::readmatrix()
{
    if(N != 0 && K!=0 && K < N)
    {
        //read x, y matrixes from input file
        for(int k = 0; k < N; k++)
        {
            x[k] = k+1;
            y[k] = data.at(k);
        }

        //init square sums matrix
        for(int i = 0; i < K+1; i++)
        {
            for(int j = 0; j < K+1; j++)
            {
                sums[i][j] = 0;
                for(int k = 0; k < N; k++)
                {
                    sums[i][j] += pow(x[k], i+j);
                }
            }
        }

        //init free coefficients column
        for(int i=0; i<K+1; i++)
        {
            for(int k=0; k<N; k++)
            {
               b[i] += pow(x[k], i) * y[k];
            }
        }
    }
    else
    {
        QString errorString = "N is not defined";
        qDebug() << errorString;
    }

}

void Polyfuntions::diagonal()
{
    double temp = 0;

   for(int i=0; i<K+1; i++)
   {
       if(sums[i][i] == 0)
       {
           for(int j=0; j<K+1; j++)
           {
               if(j == i)
                   continue;

               if(sums[j][i] != 0 && sums[i][j] != 0)
               {
                   for(int k = 0; k < K+1; k++)
                   {
                       temp = sums[j][k];
                       sums[j][k] = sums[i][k];
                       sums[i][k] = temp;
                   }

                   temp = b[j];
                   b[j] = b[i];
                   b[i] = temp;

                   break;
               }
           }
       }
   }
}

void Polyfuntions::processRows()
{
    if (!isCalculated)
    {
        for(int k = 0; k < K+1; k++)
        {
            for(int i = k+1; i < K+1; i++)
            {
                if(sums[k][k] == 0)
                {
                    qDebug() << "\nSolution is not exist.\n";
                    break;
                }

                double M = sums[i][k] / sums[k][k];

                for(int j = k; j < K+1; j++)
                {
                    sums[i][j] -= M * sums[k][j];
                }

                b[i] -= M*b[k];
            }
        }

        for(int i=(K+1)-1; i >=0; i--)
        {
            double s = 0;

            for(int j = i; j < K+1; j++)
            {
                s = s + sums[i][j]*a[j];
            }

            a[i] = (b[i] - s) / sums[i][i];
        }

        isCalculated = true;
    }

    else
    {
        qDebug() << "coefficients are already calculated";
    }
}

QList<double> Polyfuntions::getCoefficients()
{
    QList<double> results;

    if(isCalculated)
    {
        for(int i = 0; i < K+1; i++)
        {
          //  qDebug() << a[i] << endl;
            results.append(a[i]);
        }

        return results;
    }

    qDebug() << "No results" << endl;
    return results;
}

void Polyfuntions::freematrix()
{
   //free memory for matrixes
   for(int i=0; i < K+1; i++)
   {
       delete [] sums[i];
   }
   delete [] a;
   delete [] b;
   delete [] x;
   delete [] y;
   delete [] sums;
   isCalculated = false;
}

void Polyfuntions::printmatrix()
{
   //print matrix "a"
    for (int i = 0; i < 3; i++)
    {
        cout << endl;
    }

   for(int i = 0; i < K+1; i++)
   {
       for(int j = 0; j < K+1; j++)
       {
           cout << "\t"  << setprecision(5) << sums[i][j];
       }
       cout << "\t"  << setprecision(5) << b[i] << endl;

   }

}

void Polyfuntions::calculate()
{
    allocmatrix();
    readmatrix();
  //  printmatrix();
    diagonal();
 //   printmatrix();
    processRows();
   // printmatrix();
}

QList<double> Polyfuntions::getPolinomValues()
{
    QList<double> polinomValues;
    double value;

    for(int i = 1; i < data.size()+1; i++)
    {
       value = (qPow(i,5) * a[5])
                      +(qPow(i,4)*a[4])
                      +(qPow(i,3)*a[3])
                      +(qPow(i,2)*a[2])
                      +(i*a[1])
                      +a[0];

       polinomValues.append(value);
    }

    return polinomValues;
}



// TODO

//void calculateask(){
//    //Вычисляем коэффициенты полинома первой степениa[1]= 1;
//    a[0] = -x[0];//цикл по числу полиномов
//    for(int k=2;k<=n; k++){ //Вычисляем коэффициенты полинома степени k //Вначале старший коэффициент
//        a[k]= a[k-1]; //затем остальные коэффициенты, кроме последнего
//        for(int i=k-1;i>0; i--) { a[i] = a[i-1]- a[i]*x[k-1]; } //теперь младший коэффициент
//        a[0]= -a[0]*x[k-1];}//Последний этап - умножение коэффициентов на an
//    for(int i=0; i<=n; i++) a[i] = a[i]*an;
//}



