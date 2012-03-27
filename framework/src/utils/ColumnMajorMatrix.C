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
extern "C" void dsyev_ ( ... );
extern "C" void dgetri_ ( ... );
extern "C" void dgetrf_ ( ... );

ColumnMajorMatrix::ColumnMajorMatrix(unsigned int rows, unsigned int cols)
  : _n_rows(rows),
    _n_cols(cols),
    _n_entries(rows*cols),
    _values(rows*cols, 0.0)
{
  _values.resize(rows*cols);
}

ColumnMajorMatrix::ColumnMajorMatrix(const ColumnMajorMatrix &rhs)
  : _n_rows(LIBMESH_DIM),
    _n_cols(LIBMESH_DIM),
    _n_entries(_n_cols*_n_cols)
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
  : _n_rows(LIBMESH_DIM),
    _n_cols(LIBMESH_DIM),
    _n_entries(_n_cols*_n_cols)
{
   *this = rhs;
}

ColumnMajorMatrix::ColumnMajorMatrix(const DenseVector<Real> &rhs)
: _n_rows(LIBMESH_DIM),
  _n_cols(LIBMESH_DIM),
  _n_entries(_n_cols*_n_cols)
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
  mooseAssert(_n_rows == rhs.m(), "different number of rows");
  mooseAssert(_n_cols == rhs.n(), "different number of cols");

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
  mooseAssert(_n_rows == rhs.size(), "different number of rows");
  mooseAssert(_n_cols == 1, "different number of cols");

   _n_rows = rhs.size();
   _n_cols = 1;
   _n_entries = rhs.size();
   _values.resize(rhs.size());


    for (unsigned int i=0; i<_n_rows; ++i)
      (*this)(i) = rhs(i);

    return *this;
}



void
ColumnMajorMatrix::eigen(ColumnMajorMatrix & eval, ColumnMajorMatrix & evec) const
{
   mooseAssert(_n_rows == _n_cols, "Cannot solve eigen system of a non-square matrix!");

  char jobz = 'V';
  char uplo = 'U';
  int n = _n_rows;
  int buffer_size = -1;
//   Real opt_buffer_size;
//   Real *buffer;
  int return_value = 0;

  eval._n_rows = _n_rows;
  eval._n_cols = 1;
  eval._n_entries = _n_rows;
  eval._values.resize(_n_rows);

  evec = *this;

  Real * eval_data = eval.rawData();
  Real * evec_data = evec.rawData();


/*
  dsyev_(&jobz, &uplo, &n, a_data, &n, w_data, &opt_buffer_size, &buffer_size, &return_value);

  if (return_value)
    mooseError("");

  buffer_size = (int) opt_buffer_size;

  buffer = new Real[buffer_size];

  dsyev_(&jobz, &uplo, &n, a_data, &n, w_data, buffer, &buffer_size, &return_value);
  delete [] buffer;

  if (return_value)
  mooseError("");*/


  buffer_size = n * 64;
  ColumnMajorMatrix buffer(buffer_size,1);
  Real * b_data = buffer.rawData();

  dsyev_(&jobz, &uplo, &n, evec_data, &n, eval_data, b_data, &buffer_size, &return_value);

  if (return_value)
    mooseError("error in lapack eigen solve");


}




void
ColumnMajorMatrix::inverse(ColumnMajorMatrix & invA) const
{
  mooseAssert(_n_rows == _n_cols, "Cannot solve for inverse of a non-square matrix!");
  mooseAssert(_n_rows == invA._n_cols && _n_cols == invA._n_rows, "Matrices must be the same shape for matrix inverse!");

  int n = _n_rows;
  int buffer_size = -1;
  int return_value = 0;

  invA = *this;

  std::vector<int> ipiv(n);


  Real * invA_data = invA.rawData();

  buffer_size = n * 64;
  ColumnMajorMatrix buffer(buffer_size,1);
  Real * b_data = buffer.rawData();

  dgetrf_(&n, &n, invA_data, &n, &ipiv[0], &return_value);

  dgetri_(&n, invA_data, &n, &ipiv[0], b_data, &buffer_size, &return_value);

  if (return_value)
    mooseError("error in lapack inverse solve");

}
