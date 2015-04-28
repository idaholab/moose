#ifndef XFEMLIGHTSOLVER_H
#define XFEMLIGHTSOLVER_H

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include <stdio.h>
#include <iostream>
#include <cmath>

using namespace libMesh;

static Real **openMatrix(int m, int n)
{
  Real **A;
  A = new Real*[m];
  for (unsigned int i = 0; i < m; ++i)
    A[i]=new Real[n];
  return(A);
}

static void deleteMatrix(Real **A, int m)
{
  for (unsigned int i = 0; i < m; ++i)
    delete []A[i];
  delete []A;
}

static int LUfactor(Real **A,int *P,int n) /* factor PA=LU WITH PIVOTING */
{
/* HINT :  A[P[i]][j]...... */

        int i,j,k,iMax,temp;
        Real Ajj,Lij;

        for(j = 0; j <= n-2; ++j)                     /* go column by column j=0..n-2!! */
        {
                Ajj = A[P[j]][j];
                iMax = j;
                for(i = j+1; i < n; ++i)
                {
                        if(fabs(A[P[i]][j]) > fabs(Ajj))
                        {
                                Ajj = A[P[i]][j];  /*searhing for the max Ajj*/
                                iMax = i;
                        }
                }

                temp = P[j];
                P[j] = P[iMax];
                P[iMax] = temp;

                if(fabs(Ajj) < 1e-15)
                {
                        printf("ZERO Diagonal element zero -- LU fails\n");
                        return(-1); /* error signal ! */
                }
                for(i = j+1; i < n; ++i)              /* zero-out sub-diag rows */
                {
                        Lij = A[P[i]][j]/Ajj;     /* calculate L-factors */
                        A[P[i]][j] = Lij;         /* store Lij entries */
                        for(k = j+1; k < n; ++k)      /* row subtraction */
                                A[P[i]][k] -= Lij*A[P[j]][k];
                }
        }
        return(0); /* no error */
}

static void LUsolve(Real **A, int *P, Real *b, int n)/*PA=L\U,LUx=Pb,Ly=Pb,Ux=y*/
{
/* HINT: b[P[i]].... */

        Real **L,**U;
        Real *x,*y;
        Real rowsum;
        int i,j;

        y = x = b;          /* aliases, alternate names for the same vector */
        L = U = A;          /* aliases for parts of the A matrix=L\U */
        y[P[0]] = b[P[0]];                        /* forward substitution Ly=b */
        for(i = 1; i < n; ++i)
        {
                y[P[i]] = b[P[i]];
                rowsum = 0.0;
                for(j = 0;j < i; ++j)
                        rowsum += L[P[i]][j]*y[P[j]];
                y[P[i]] -= rowsum;
        }
        x[P[n-1]] = y[P[n-1]]/U[P[n-1]][n-1];     /* back substitution Ux=y */
        for(i = n-2;i >= 0; --i)
        {
                rowsum = 0.0;
                for(j = n-1; j > i; --j)
                        rowsum += U[P[i]][j]*x[P[j]];
                x[P[i]] = (y[P[i]] - rowsum)/U[P[i]][i];
        }
}

static void solveLinearSystLU(Real **A, Real *b, int n) // ZZY solver
{
  // Get row index for pivoting

  int *P;
  P = new int[n];
  for (unsigned int i = 0; i < n; ++i)
    P[i] = i;

  // Call LU decomposition and solving routines

  LUfactor(A,P,n);
  LUsolve(A,P,b,n);

  // Get the solution and store it in b

  Real *x;
  x = new Real[n];
  for (unsigned int i = 0; i < n; ++i)
    x[i] = b[P[i]];
  for (unsigned int i = 0; i < n; ++i)
    b[i] = x[i];

  // delete arrays

  delete[] P;
  delete[] x;

}


#endif
