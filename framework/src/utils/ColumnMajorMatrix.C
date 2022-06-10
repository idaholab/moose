//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ColumnMajorMatrix.h"

#include "DualRealOps.h"

#include "libmesh/petsc_macro.h"

// PETSc includes
#include <petscsys.h>
#include <petscblaslapack.h>

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(unsigned int rows, unsigned int cols)
  : _n_rows(rows), _n_cols(cols), _n_entries(rows * cols), _values(rows * cols, 0.0)
{
  _values.resize(rows * cols);
}

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(const ColumnMajorMatrixTempl<T> & rhs)
  : _n_rows(LIBMESH_DIM), _n_cols(LIBMESH_DIM), _n_entries(_n_cols * _n_cols)
{
  *this = rhs;
}

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(const TypeTensor<T> & rhs)
  : _n_rows(LIBMESH_DIM),
    _n_cols(LIBMESH_DIM),
    _n_entries(LIBMESH_DIM * LIBMESH_DIM),
    _values(LIBMESH_DIM * LIBMESH_DIM)
{
  for (const auto j : make_range(Moose::dim))
    for (const auto i : make_range(Moose::dim))
      (*this)(i, j) = rhs(i, j);
}

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(const DenseMatrix<T> & rhs)
  : _n_rows(LIBMESH_DIM), _n_cols(LIBMESH_DIM), _n_entries(_n_cols * _n_cols)
{
  *this = rhs;
}

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(const DenseVector<T> & rhs)
  : _n_rows(LIBMESH_DIM), _n_cols(LIBMESH_DIM), _n_entries(_n_cols * _n_cols)
{
  *this = rhs;
}

template <typename T>
ColumnMajorMatrixTempl<T>::ColumnMajorMatrixTempl(const TypeVector<T> & col1,
                                                  const TypeVector<T> & col2,
                                                  const TypeVector<T> & col3)
  : _n_rows(LIBMESH_DIM),
    _n_cols(LIBMESH_DIM),
    _n_entries(LIBMESH_DIM * LIBMESH_DIM),
    _values(LIBMESH_DIM * LIBMESH_DIM)
{
  unsigned int entry = 0;
  for (const auto i : make_range(Moose::dim))
    _values[entry++] = col1(i);

  for (const auto i : make_range(Moose::dim))
    _values[entry++] = col2(i);

  for (const auto i : make_range(Moose::dim))
    _values[entry++] = col3(i);
}

template <typename T>
ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::kronecker(const ColumnMajorMatrixTempl<T> & rhs) const
{
  rhs.checkSquareness();

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows * rhs._n_rows, _n_cols * rhs._n_cols);

  for (unsigned int i = 0; i < _n_rows; i++)
    for (unsigned int j = 0; j < _n_cols; j++)
      for (unsigned int k = 0; k < rhs._n_rows; k++)
        for (unsigned int l = 0; l < rhs._n_cols; l++)
          ret_matrix(((i * _n_rows) + k), ((j * _n_cols) + l)) = (*this)(i, j) * rhs(k, l);

  return ret_matrix;
}

template <typename T>
ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator=(const DenseMatrix<T> & rhs)
{
  if (_n_rows != rhs.m() || _n_cols != rhs.n())
    mooseError("ColumnMajorMatrix and DenseMatrix should be of the same shape.");

  _n_rows = rhs.m();
  _n_cols = rhs.n();
  _n_entries = rhs.m() * rhs.n();
  _values.resize(rhs.m() * rhs.n());

  for (unsigned int j = 0; j < _n_cols; ++j)
    for (unsigned int i = 0; i < _n_rows; ++i)
      (*this)(i, j) = rhs(i, j);

  return *this;
}

template <typename T>
ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator=(const DenseVector<T> & rhs)
{
  if (_n_rows != rhs.size() || _n_cols != 1)
    mooseError("ColumnMajorMatrix and DenseVector should be of the same shape.");

  _n_rows = rhs.size();
  _n_cols = 1;
  _n_entries = rhs.size();
  _values.resize(rhs.size());

  for (unsigned int i = 0; i < _n_rows; ++i)
    (*this)(i) = rhs(i);

  return *this;
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::eigen(ColumnMajorMatrixTempl<T> & eval,
                                 ColumnMajorMatrixTempl<T> & evec) const
{
  this->checkSquareness();

  char jobz = 'V';
  char uplo = 'U';
  PetscBLASInt n = _n_rows;
  PetscBLASInt return_value = 0;

  eval._n_rows = _n_rows;
  eval._n_cols = 1;
  eval._n_entries = _n_rows;
  eval._values.resize(_n_rows);

  evec = *this;

  T * eval_data = eval.rawData();
  T * evec_data = evec.rawData();

  PetscBLASInt buffer_size = n * 64;
  std::vector<T> buffer(buffer_size);

  LAPACKsyev_(&jobz, &uplo, &n, evec_data, &n, eval_data, &buffer[0], &buffer_size, &return_value);

  if (return_value)
    mooseError("error in lapack eigen solve");
}

template <>
void
ColumnMajorMatrixTempl<DualReal>::eigen(ColumnMajorMatrixTempl<DualReal> &,
                                        ColumnMajorMatrixTempl<DualReal> &) const
{
  mooseError("Eigen solves with AD types is not supported.");
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::eigenNonsym(ColumnMajorMatrixTempl<T> & eval_real,
                                       ColumnMajorMatrixTempl<T> & eval_img,
                                       ColumnMajorMatrixTempl<T> & evec_right,
                                       ColumnMajorMatrixTempl<T> & evec_left) const
{
  this->checkSquareness();

  ColumnMajorMatrixTempl<T> a(*this);

  char jobvl = 'V';
  char jobvr = 'V';
  PetscBLASInt n = _n_rows;
  PetscBLASInt return_value = 0;

  eval_real._n_rows = _n_rows;
  eval_real._n_cols = 1;
  eval_real._n_entries = _n_rows;
  eval_real._values.resize(_n_rows);

  eval_img._n_rows = _n_rows;
  eval_img._n_cols = 1;
  eval_img._n_entries = _n_rows;
  eval_img._values.resize(_n_rows);

  T * a_data = a.rawData();
  T * eval_r = eval_real.rawData();
  T * eval_i = eval_img.rawData();
  T * evec_ri = evec_right.rawData();
  T * evec_le = evec_left.rawData();

  PetscBLASInt buffer_size = n * 64;
  std::vector<T> buffer(buffer_size);

  LAPACKgeev_(&jobvl,
              &jobvr,
              &n,
              a_data,
              &n,
              eval_r,
              eval_i,
              evec_le,
              &n,
              evec_ri,
              &n,
              &buffer[0],
              &buffer_size,
              &return_value);

  if (return_value)
    mooseError("error in lapack eigen solve");
}

template <>
void
ColumnMajorMatrixTempl<DualReal>::eigenNonsym(ColumnMajorMatrixTempl<DualReal> &,
                                              ColumnMajorMatrixTempl<DualReal> &,
                                              ColumnMajorMatrixTempl<DualReal> &,
                                              ColumnMajorMatrixTempl<DualReal> &) const
{
  mooseError("Eigen solves with AD types is not supported.");
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::exp(ColumnMajorMatrixTempl<T> & z) const
{
  this->checkSquareness();

  ColumnMajorMatrixTempl<T> a(*this);
  ColumnMajorMatrixTempl<T> evals_real(_n_rows, 1), evals_img(_n_rows, 1),
      evals_real2(_n_rows, _n_cols);
  ColumnMajorMatrixTempl<T> evec_right(_n_rows, _n_cols), evec_left(_n_rows, _n_cols);
  ColumnMajorMatrixTempl<T> evec_right_inverse(_n_rows, _n_cols);

  a.eigenNonsym(evals_real, evals_img, evec_right, evec_left);

  for (unsigned int i = 0; i < _n_rows; i++)
    evals_real2(i, i) = std::exp(evals_real(i, 0));

  evec_right.inverse(evec_right_inverse);

  z = evec_right * evals_real2 * evec_right_inverse;
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::inverse(ColumnMajorMatrixTempl<T> & invA) const
{
  this->checkSquareness();
  this->checkShapeEquality(invA);

  PetscBLASInt n = _n_rows;
  PetscBLASInt return_value = 0;

  invA = *this;

  std::vector<PetscBLASInt> ipiv(n);
  T * invA_data = invA.rawData();

  PetscBLASInt buffer_size = n * 64;
  std::vector<T> buffer(buffer_size);

  LAPACKgetrf_(&n, &n, invA_data, &n, &ipiv[0], &return_value);

  LAPACKgetri_(&n, invA_data, &n, &ipiv[0], &buffer[0], &buffer_size, &return_value);

  if (return_value)
    mooseException("Error in LAPACK matrix-inverse calculation");
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::checkSquareness() const
{
  if (_n_rows != _n_cols)
    mooseError("ColumnMajorMatrix error: Unable to perform the operation on a non-square matrix.");
}

template <typename T>
void
ColumnMajorMatrixTempl<T>::checkShapeEquality(const ColumnMajorMatrixTempl<T> & rhs) const
{
  if (_n_rows != rhs._n_rows || _n_cols != rhs._n_cols)
    mooseError("ColumnMajorMatrix error: Unable to perform the operation on matrices of different "
               "shapes.");
}

template <>
void
ColumnMajorMatrixTempl<DualReal>::inverse(ColumnMajorMatrixTempl<DualReal> &) const
{
  mooseError("Inverse solves with AD types is not supported for the ColumnMajorMatrix class.");
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::abs()
{
  ColumnMajorMatrixTempl<T> & s = (*this);

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols);

  for (unsigned int j = 0; j < _n_cols; j++)
    for (unsigned int i = 0; i < _n_rows; i++)
      ret_matrix(i, j) = std::abs(s(i, j));

  return ret_matrix;
}

template <typename T>
inline T
ColumnMajorMatrixTempl<T>::norm()
{
  return std::sqrt(doubleContraction(*this));
}

template class ColumnMajorMatrixTempl<Real>;
template class ColumnMajorMatrixTempl<DualReal>;
