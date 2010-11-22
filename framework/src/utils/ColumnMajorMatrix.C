/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ColumnMajorMatrix.h"
#include "c++eigen.h"

ColumnMajorMatrix::ColumnMajorMatrix(unsigned int rows, unsigned int cols)
  : _n_rows(rows),
    _n_cols(cols),
    _n_entries(rows*cols)
{
  _values.resize(rows*cols);
}

ColumnMajorMatrix::ColumnMajorMatrix(const ColumnMajorMatrix &rhs)
{
  *this = rhs;
}

ColumnMajorMatrix::ColumnMajorMatrix(const TypeTensor<Real> &rhs)
  :_n_rows(LIBMESH_DIM),
   _n_cols(LIBMESH_DIM),
   _n_entries(LIBMESH_DIM*LIBMESH_DIM),
   _values(LIBMESH_DIM*LIBMESH_DIM)
{
  for (unsigned int j=0; j<LIBMESH_DIM; ++j)
    for (unsigned int i=0; i<LIBMESH_DIM; ++i)
      (*this)(i, j) = rhs(i, j);
}

ColumnMajorMatrix::ColumnMajorMatrix(const DenseMatrix<Real> &rhs)
{
   *this = rhs;
}

ColumnMajorMatrix::ColumnMajorMatrix(const DenseVector<Real> &rhs)
{
   *this = rhs;
}

ColumnMajorMatrix::ColumnMajorMatrix(const TypeVector<Real> & col1, const TypeVector<Real> & col2, const TypeVector<Real> & col3)
  :_n_rows(LIBMESH_DIM),
   _n_cols(LIBMESH_DIM),
   _n_entries(LIBMESH_DIM*LIBMESH_DIM),
   _values(LIBMESH_DIM*LIBMESH_DIM)
{
  unsigned int entry = 0;
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    _values[entry++] = col1(i);

  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    _values[entry++] = col2(i);

  for(unsigned int i=0; i<LIBMESH_DIM; i++)
    _values[entry++] = col3(i);
}


ColumnMajorMatrix ColumnMajorMatrix::kronecker  (const ColumnMajorMatrix &  rhs) const  
 {
   mooseAssert(_n_rows == rhs._n_cols && _n_cols == rhs._n_rows, "Matrices must be the same shape for a kronecker product!");

   ColumnMajorMatrix ret_matrix(_n_rows*_n_rows, rhs._n_cols*rhs._n_cols);
     
  for(unsigned int i = 0; i < _n_rows; i++)
    for(unsigned int j = 0; j < _n_cols; j++)
      for (unsigned int k = 0; k < rhs._n_rows; k++)
        for(unsigned int l = 0; l < rhs._n_cols; l++)
          ret_matrix(((i*_n_rows)+k),((j*_n_cols)+l)) = (*this)(i,j) * rhs(k,l);
      
   return ret_matrix;
}





ColumnMajorMatrix & ColumnMajorMatrix::operator=(const DenseMatrix<Real> &rhs)
 {

  _n_rows = rhs.m();
  _n_cols = rhs.n();
  _n_entries = rhs.m()*rhs.n();
  _values.resize(rhs.m()*rhs.n());
  
  for (unsigned int j=0; j<_n_cols; ++j)
    for (unsigned int i=0; i<_n_rows; ++i)
         (*this)(i, j) = rhs(i, j);
   
    return *this;
}


ColumnMajorMatrix & ColumnMajorMatrix::operator=(const DenseVector<Real> &rhs)
 {

   _n_rows = rhs.size();
   _n_cols = 1;
   _n_entries = rhs.size();
   _values.resize(rhs.size());
  
 
    for (unsigned int i=0; i<_n_rows; ++i)
      (*this)(i) = rhs(i);

    return *this;
}



void ColumnMajorMatrix::eigen  (ColumnMajorMatrix &  d_matrix, ColumnMajorMatrix &  a_matrix) const
 {

    mooseAssert(_n_rows == _n_cols, "Matrices must be square for a eigen system to solve!");
    
   const int ND = _n_cols;
   double **a_mat, dummy;
   double *d_mat,*e_mat, *e;
   
   a_matrix._n_cols = ND;
   a_matrix._n_rows = ND;
   a_matrix._n_entries = _n_entries;
   a_matrix._values.resize(_n_entries);
   
   d_matrix._n_cols = 1;
   d_matrix._n_rows = ND;
   d_matrix._n_entries = _n_cols;
   d_matrix._values.resize(_n_cols);
   
   a_mat = dmatrix(1,ND,1,ND);
   d_mat = dvector(1,ND);
   e_mat = dvector(1,ND);
   e = dvector(1,ND);
   
   for( int j=1;j<=ND;j++)
     for( int i=1;i<=ND;i++)
       a_mat[i][j] = this->_values[(j-1)*_n_cols+(i-1)];
   

   
   tred2(a_mat, ND, d_mat, e_mat);
   tqli(d_mat, e_mat, ND, a_mat);
   
   /* Sort the eigenvalues in ascending order */
   for (int i=1; i<ND; i++)
     for (int j=i+1; j<=ND; j++)
       if (d_mat[i]>d_mat[j]) {
         dummy = d_mat[i];
         d_mat[i] = d_mat[j];
         d_mat[j] = dummy;
         for (int k=1; k<=ND; k++) e[k] = a_mat[k][i];
         for (int k=1; k<=ND; k++) a_mat[k][i] = a_mat[k][j];
         for (int k=1; k<=ND; k++) a_mat[k][j] = e[k];
       }
   
   for( int j=1;j<=ND;j++)
   {
     d_matrix._values[j-1] = d_mat[j];
     
     for( int i=1;i<=ND;i++)
       a_matrix._values[(j-1)*_n_cols+(i-1)] = a_mat[i][j];  
   }
   
        
   
   
 }


