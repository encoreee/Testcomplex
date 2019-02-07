#include <stdio.h>
#include <process.h>
#include <math.h>
#include <polyfunctions.h>
#include <QDebug>

void Polyfuntions::setInputData(QList<double> inputData)
{
    data = inputData;
    N = data.size();
}

void Polyfuntions::setPolyPower(int K)
{
    this->K = K;
}

void Polyfuntions::allocmatrix()
{
   //allocate memory for matrixes

   a = new float[K+1];
   b = new float[K+1];
   x = new float[N];
   y = new float[N];

   sums = new float*[K+1];


   if(x==nullptr || y==nullptr || a==nullptr || sums==nullptr)
   {
       QString errorString = QString("\nNot enough memory to allocate. N=%1, K=%2\n").arg(N).arg(K);
       qDebug() << errorString;
   }


   for(int i = 0; i < K + 1; i++)
   {
       sums[i] = new float[K+1];
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

void diagonal()
{
   int i, j, k;
   float temp=0;
   for(i=0; i<K+1; i++)
   {
       if(sums[i][i]==0)
       {
           for(j=0; j<K+1; j++)
           {
               if(j==i) continue;
               if(sums[j][i] !=0 && sums[i][j]!=0)
               {
                   for(k=0; k<K+1; k++)
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

void processRows()
{
    int i=0,j=0, k=0;
    for(k=0; k<K+1; k++){
           for(i=k+1; i<K+1; i++){
           if(sums[k][k]==0){
               printf("\nSolution is not exist.\n");
               return;
           }
           float M = sums[i][k] / sums[k][k];
           for(j=k; j<K+1; j++){
               sums[i][j] -= M * sums[k][j];
           }
           b[i] -= M*b[k];
           }
       }
}


QList<double> getresult()
{
   QList<double> results;
   for(int i=0; i<K+1; i++)
   {
       printf("a[%d] = %f\n", i, a[i]);
       results.append(a[i]);
   }
   return results;
}

void freematrix()
{
   //free memory for matrixes
   for(int i=0; i<K+1; i++)
   {
       delete [] sums[i];
   }
   delete [] a;
   delete [] b;
   delete [] x;
   delete [] y;
   delete [] sums;
}


