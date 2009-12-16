#include "ColumnMajorMatrix.h"

ColumnMajorMatrix::ColumnMajorMatrix(unsigned int rows, unsigned int cols)
  : _n_rows(rows),
    _n_cols(cols),
    _values(rows*cols)
{}

inline
ColumnMajorMatrix::ColumnMajorMatrix(const ColumnMajorMatrix &rhs)
{
  *this = rhs;
}


ColumnMajorMatrix::ColumnMajorMatrix(const TypeTensor<Real> &rhs)
  :_n_rows(LIBMESH_DIM),
   _n_cols(LIBMESH_DIM),
   _values(LIBMESH_DIM*LIBMESH_DIM)
{
  for (unsigned int j=0; j<LIBMESH_DIM; ++j)
    for (unsigned int i=0; i<LIBMESH_DIM; ++i)
      (*this)(i, j) = rhs(i, j);
}

ColumnMajorMatrix::ColumnMajorMatrix(const TypeVector<Real> & col1, const TypeVector<Real> & col2, const TypeVector<Real> & col3)
{
  unsigned int entry = 0;
  for(unsigned int i=0; i<LIBMESH_DIM; i++)
  {
    _values[entry] = col1(i);
    entry++;
  }

  for(unsigned int i=0; i<LIBMESH_DIM; i++)
  {
    _values[entry] = col2(i);
    entry++;
  }

  for(unsigned int i=0; i<LIBMESH_DIM; i++)
  {
    _values[entry] = col3(i);
    entry++;
  }
}

    
