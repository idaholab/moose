//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "MooseError.h"
#include "ADReal.h"
#include "MooseTypes.h"

#include "libmesh/type_tensor.h"
#include "libmesh/dense_vector.h"

#include "metaphysicl/raw_type.h"

// C++ includes
#include <iomanip>

/**
 * This class defines a Tensor that can change its shape.  This means
 * a 3x3x3x3 Tensor can be represented as a 9x9 or an 81x1.  Further,
 * the values of this tensor are _COLUMN_ major ordered!
 */
template <typename T>
class ColumnMajorMatrixTempl
{
public:
  /**
   * Constructor that sets an initial number of entries and shape.
   * Defaults to creating the same size tensor as TensorValue
   */
  explicit ColumnMajorMatrixTempl(const unsigned int rows = Moose::dim,
                                  const unsigned int cols = Moose::dim);

  /**
   * Copy Constructor defined in terms of operator=()
   */
  ColumnMajorMatrixTempl(const ColumnMajorMatrixTempl<T> & rhs);

  /**
   * Constructor that fills in the ColumnMajorMatrixTempl with values from a libMesh TypeTensor
   */
  explicit ColumnMajorMatrixTempl(const TypeTensor<T> & tensor);

  explicit ColumnMajorMatrixTempl(const DenseMatrix<T> & rhs);

  explicit ColumnMajorMatrixTempl(const DenseVector<T> & rhs);

  /**
   * Constructor that takes in 3 vectors and uses them to create columns
   */
  ColumnMajorMatrixTempl(const TypeVector<T> & col1,
                         const TypeVector<T> & col2,
                         const TypeVector<T> & col3);

  /**
   * The total number of entries in the Tensor.
   * i.e. cols * rows
   */
  unsigned int numEntries() const;

  /**
   * Change the shape of the tensor.
   * Note that cols * rows should be equal to numEntries()!
   */
  void reshape(const unsigned int rows, const unsigned int cols);

  /**
   * Get the i,j entry
   * j defaults to zero so you can use it as a column vector.
   */
  T & operator()(const unsigned int i, const unsigned int j = 0);

  /**
   * Get the i,j entry
   *
   * j defaults to zero so you can use it as a column vector.
   * This is the version used for a const ColumnMajorMatrixTempl.
   */
  T operator()(const unsigned int i, const unsigned int j = 0) const;

  /**
   * Print the tensor
   */
  void print();

  /**
   * Prints to file
   */
  void print_scientific(std::ostream & os);

  /**
   * Fills the passed in tensor with the values from this tensor.
   */
  void fill(TypeTensor<T> & tensor);

  /**
   * Fills the passed in dense matrix with the values from this tensor.
   */
  void fill(DenseMatrix<T> & rhs);

  /**
   * Fills the passed in dense vector with the values from this tensor.
   */
  void fill(DenseVector<T> & rhs);

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on.
   */
  ColumnMajorMatrixTempl<T> transpose() const;

  /**
   * Returns a matrix that is the deviatoric of the matrix this
   * was called on.
   */
  ColumnMajorMatrixTempl<T> deviatoric();

  /**
   * Returns a matrix that is the absolute value of the matrix this
   * was called on.
   */
  ColumnMajorMatrixTempl<T> abs();

  /**
   * Set the value of each of the diagonals to the passed in value.
   */
  void setDiag(T value);

  /**
   * Add to each of the diagonals the passsed in value.
   */
  void addDiag(T value);

  /**
   * The trace of the CMM.
   */
  T tr() const;

  /**
   * Zero the matrix.
   */
  void zero();

  /**
   * Turn the matrix into an identity matrix.
   */
  void identity();

  /**
   * Double contraction of two matrices ie A : B = Sum(A_ab * B_ba)
   */
  T doubleContraction(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * The Euclidean norm of the matrix.
   */
  T norm();

  /**
   * Returns the number of rows
   */
  unsigned int n() const;

  /**
   * Returns the number of columns
   */
  unsigned int m() const;

  /**
   * Returns eigen system solve for a symmetric real matrix
   */
  void eigen(ColumnMajorMatrixTempl<T> & eval, ColumnMajorMatrixTempl<T> & evec) const;

  /**
   * Returns eigen system solve for a non-symmetric real matrix
   */
  void eigenNonsym(ColumnMajorMatrixTempl<T> & eval_real,
                   ColumnMajorMatrixTempl<T> & eval_img,
                   ColumnMajorMatrixTempl<T> & evec_right,
                   ColumnMajorMatrixTempl<T> & eve_left) const;

  /**
   * Returns matrix that is the exponential of the matrix this was called on
   */
  void exp(ColumnMajorMatrixTempl<T> & z) const;

  /**
   * Returns inverse of a general matrix
   */
  void inverse(ColumnMajorMatrixTempl<T> & invA) const;

  /**
   * Returns a reference to the raw data pointer
   */
  T * rawData();
  const T * rawData() const;

  /**
   * Kronecker Product
   */
  ColumnMajorMatrixTempl<T> kronecker(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * Sets the values in _this_ tensor to the values on the RHS.
   * Will also reshape this tensor if necessary.
   */
  ColumnMajorMatrixTempl<T> & operator=(const TypeTensor<T> & rhs);

  /**
   * Sets the values in _this_ dense matrix to the values on the RHS.
   * Will also reshape this tensor if necessary.
   */
  ColumnMajorMatrixTempl<T> & operator=(const DenseMatrix<T> & rhs);

  /**
   * Sets the values in _this_ dense vector to the values on the RHS.
   * Will also reshape this tensor if necessary.
   */
  ColumnMajorMatrixTempl<T> & operator=(const DenseVector<T> & rhs);

  /**
   * Sets the values in _this_ tensor to the values on the RHS
   * Will also reshape this tensor if necessary.
   */
  template <typename T2>
  ColumnMajorMatrixTempl<T> & operator=(const ColumnMajorMatrixTempl<T2> & rhs);

  /**
   * defaulted operator=
   */
  ColumnMajorMatrixTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & rhs) = default;

  /**
   * Scalar multiplication of the ColumnMajorMatrixTempl
   */
  ColumnMajorMatrixTempl<T> operator*(T scalar) const;

  /**
   * Matrix Vector Multiplication of the libMesh TypeVector Type
   */
  ColumnMajorMatrixTempl<T> operator*(const TypeVector<T> & rhs) const;

  //   /**
  //    * Matrix Vector Multiplication of the TypeTensor Product.  Note that the
  //    * Tensor type is treated as a single dimension Vector for this operation
  //    */
  //   ColumnMajorMatrixTempl operator*(const TypeTensor<T> & rhs) const;

  /**
   * Matrix Matrix Multiplication
   */
  ColumnMajorMatrixTempl<T> operator*(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * Matrix Matrix Addition
   */
  ColumnMajorMatrixTempl<T> operator+(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * Matrix Matrix Subtraction
   */
  ColumnMajorMatrixTempl<T> operator-(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * Matrix Matrix Addition plus assignment
   *
   * Note that this is faster than regular addition
   * because the result doesn't have to get copied out
   */
  ColumnMajorMatrixTempl<T> & operator+=(const ColumnMajorMatrixTempl<T> & rhs);

  /**
   * Matrix Tensor Addition Plus Assignment
   */
  ColumnMajorMatrixTempl<T> & operator+=(const TypeTensor<T> & rhs);

  /**
   * Matrix Matrix Subtraction plus assignment
   *
   * Note that this is faster than regular subtraction
   * because the result doesn't have to get copied out
   */
  ColumnMajorMatrixTempl<T> & operator-=(const ColumnMajorMatrixTempl<T> & rhs);

  /**
   * Scalar addition
   */
  ColumnMajorMatrixTempl<T> operator+(T scalar) const;

  /**
   * Scalar Multiplication plus assignment
   */
  ColumnMajorMatrixTempl<T> & operator*=(T scalar);

  /**
   * Scalar Division plus assignment
   */
  ColumnMajorMatrixTempl<T> & operator/=(T scalar);

  /**
   * Scalar Addition plus assignment
   */
  ColumnMajorMatrixTempl<T> & operator+=(T scalar);

  /**
   * Check if matrix is square
   */
  void checkSquareness() const;

  /**
   * Check if matrices are of same shape
   */
  void checkShapeEquality(const ColumnMajorMatrixTempl<T> & rhs) const;

  /**
   * Equality operators
   */
  bool operator==(const ColumnMajorMatrixTempl<T> & rhs) const;
  bool operator!=(const ColumnMajorMatrixTempl<T> & rhs) const;

protected:
  unsigned int _n_rows, _n_cols, _n_entries;
  std::vector<T> _values;

  template <typename T2>
  friend void dataStore(std::ostream &, ColumnMajorMatrixTempl<T2> &, void *);
  template <typename T2>
  friend void dataLoad(std::istream &, ColumnMajorMatrixTempl<T2> &, void *);
};

template <typename T>
inline unsigned int
ColumnMajorMatrixTempl<T>::numEntries() const
{
  return _n_entries;
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::reshape(unsigned int rows, unsigned int cols)
{
  if (cols * rows == _n_entries)
  {
    _n_rows = rows;
    _n_cols = cols;
  }
  else
  {
    _n_rows = rows;
    _n_cols = cols;
    _n_entries = _n_rows * _n_cols;
    _values.resize(_n_entries);
  }
}

template <typename T>
inline T &
ColumnMajorMatrixTempl<T>::operator()(const unsigned int i, const unsigned int j)
{
  if ((i * j) >= _n_entries)
    mooseError("Reference outside of ColumnMajorMatrix bounds!");

  // Row major indexing!
  return _values[(j * _n_rows) + i];
}

template <typename T>
inline T
ColumnMajorMatrixTempl<T>::operator()(const unsigned int i, const unsigned int j) const
{
  if ((i * j) >= _n_entries)
    mooseError("Reference outside of ColumnMajorMatrix bounds!");

  // Row major indexing!
  return _values[(j * _n_rows) + i];
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::print()
{
  ColumnMajorMatrixTempl<T> & s = (*this);

  for (unsigned int i = 0; i < _n_rows; i++)
  {
    for (unsigned int j = 0; j < _n_cols; j++)
      Moose::out << std::setw(15) << s(i, j) << " ";

    Moose::out << std::endl;
  }
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::print_scientific(std::ostream & os)
{
  ColumnMajorMatrixTempl<T> & s = (*this);

  for (unsigned int i = 0; i < _n_rows; i++)
  {
    for (unsigned int j = 0; j < _n_cols; j++)
      os << std::setw(15) << std::scientific << std::setprecision(8) << s(i, j) << " ";

    os << std::endl;
  }
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::fill(TypeTensor<T> & tensor)
{
  if (Moose::dim * Moose::dim != _n_entries)
    mooseError(
        "Cannot fill tensor! The ColumnMajorMatrix doesn't have the same number of entries!");

  for (const auto j : libMesh::make_range(Moose::dim))
    for (const auto i : libMesh::make_range(Moose::dim))
      tensor(i, j) = _values[j * Moose::dim + i];
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::fill(DenseMatrix<T> & rhs)
{
  if (rhs.n() * rhs.m() != _n_entries)
    mooseError(
        "Cannot fill dense matrix! The ColumnMajorMatrix doesn't have the same number of entries!");

  for (unsigned int j = 0, index = 0; j < rhs.m(); ++j)
    for (unsigned int i = 0; i < rhs.n(); ++i, ++index)
      rhs(i, j) = _values[index];
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::fill(DenseVector<T> & rhs)
{
  if (_n_rows != rhs.size() || _n_cols != 1)
    mooseError("ColumnMajorMatrix and DenseVector must be the same shape for a fill!");

  for (unsigned int i = 0; i < _n_rows; ++i)
    rhs(i) = (*this)(i);
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::transpose() const
{
  const ColumnMajorMatrixTempl<T> & s = (*this);

  ColumnMajorMatrixTempl<T> ret_matrix(_n_cols, _n_rows);

  for (unsigned int i = 0; i < _n_rows; i++)
    for (unsigned int j = 0; j < _n_cols; j++)
      ret_matrix(j, i) = s(i, j);

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::deviatoric()
{
  ColumnMajorMatrixTempl<T> & s = (*this);

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols), I(_n_rows, _n_cols);

  I.identity();

  for (unsigned int i = 0; i < _n_rows; i++)
    for (unsigned int j = 0; j < _n_cols; j++)
      ret_matrix(i, j) = s(i, j) - I(i, j) * (s.tr() / 3.0);

  return ret_matrix;
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::setDiag(T value)
{
  this->checkSquareness();

  for (unsigned int i = 0; i < _n_rows; i++)
    (*this)(i, i) = value;
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::addDiag(T value)
{
  this->checkSquareness();

  for (unsigned int i = 0; i < _n_rows; i++)
    (*this)(i, i) += value;
}

template <typename T>
inline T
ColumnMajorMatrixTempl<T>::tr() const
{
  this->checkSquareness();

  T trace = 0;

  for (unsigned int i = 0; i < _n_rows; i++)
    trace += (*this)(i, i);

  return trace;
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::zero()
{
  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] = 0;
}

template <typename T>
inline void
ColumnMajorMatrixTempl<T>::identity()
{
  this->checkSquareness();

  zero();

  for (unsigned int i = 0; i < _n_rows; i++)
    (*this)(i, i) = 1;
}

template <typename T>
inline T
ColumnMajorMatrixTempl<T>::doubleContraction(const ColumnMajorMatrixTempl<T> & rhs) const
{
  this->checkShapeEquality(rhs);

  T value = 0;

  for (unsigned int j = 0; j < _n_cols; j++)
    for (unsigned int i = 0; i < _n_rows; i++)
      value += (*this)(i, j) * rhs(i, j);

  return value;
}

template <typename T>
inline unsigned int
ColumnMajorMatrixTempl<T>::n() const
{
  return _n_rows;
}

template <typename T>
inline unsigned int
ColumnMajorMatrixTempl<T>::m() const
{
  return _n_cols;
}

template <typename T>
inline T *
ColumnMajorMatrixTempl<T>::rawData()
{
  return &_values[0];
}

template <typename T>
inline const T *
ColumnMajorMatrixTempl<T>::rawData() const
{
  return &_values[0];
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator=(const TypeTensor<T> & rhs)
{
  // Resize the tensor if necessary
  if ((Moose::dim * Moose::dim) != _n_entries)
  {
    _n_entries = Moose::dim * Moose::dim;
    _values.resize(_n_entries);
  }

  // Make sure the shape is correct
  reshape(Moose::dim, Moose::dim);

  ColumnMajorMatrixTempl<T> & s = (*this);

  // Copy the values
  for (unsigned int j = 0; j < _n_cols; j++)
    for (unsigned int i = 0; i < _n_cols; i++)
      s(i, j) = rhs(i, j);

  return *this;
}

template <typename T>
template <typename T2>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator=(const ColumnMajorMatrixTempl<T2> & rhs)
{
  this->reshape(rhs.m(), rhs.n());

  for (MooseIndex(rhs.m()) i = 0; i < rhs.m(); ++i)
    for (MooseIndex(rhs.n()) j = 0; j < rhs.n(); ++j)
      (*this)(i, j) = rhs(i, j);

  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator*(T scalar) const
{
  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols);

  for (unsigned int i = 0; i < _n_entries; i++)
    ret_matrix._values[i] = _values[i] * scalar;

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator*(const TypeVector<T> & rhs) const
{
  if (_n_cols != Moose::dim)
    mooseError("Cannot perform matvec operation! The column dimension of "
               "the ColumnMajorMatrix does not match the TypeVector!");

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, 1);

  for (unsigned int i = 0; i < _n_rows; ++i)
    for (unsigned int j = 0; j < _n_cols; ++j)
      ret_matrix._values[i] += (*this)(i, j) * rhs(j);

  return ret_matrix;
}

// template <typename T>
// inline ColumnMajorMatrixTempl<T>
// ColumnMajorMatrixTempl<T>::operator*(const TypeTensor<T> & rhs) const
// {
//   mooseAssert(_n_cols == LIBMESH_DIM*LIBMESH_DIM, "Cannot perform matvec operation!  The
//   ColumnMajorMatrixTempl<T> doesn't have the correct shape!");

//   ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, 1);

//   for (unsigned int i=0; i<_n_rows; ++i)
//     for (unsigned int j=0; j<_n_cols; ++j)
//       // Treat the Tensor as a column major column vector
//       ret_matrix._values[i] += (*this)(i, j) * rhs(j%3, j/3);

//   return ret_matrix;
// }

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator*(const ColumnMajorMatrixTempl<T> & rhs) const
{
  if (_n_cols != rhs._n_rows)
    mooseError(
        "Cannot perform matrix multiply! The shapes of the two operands are not compatible!");

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, rhs._n_cols);

  for (unsigned int i = 0; i < ret_matrix._n_rows; ++i)
    for (unsigned int j = 0; j < ret_matrix._n_cols; ++j)
      for (unsigned int k = 0; k < _n_cols; ++k)
        ret_matrix(i, j) += (*this)(i, k) * rhs(k, j);

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator+(const ColumnMajorMatrixTempl<T> & rhs) const
{
  this->checkShapeEquality(rhs);

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols);

  for (unsigned int i = 0; i < _n_entries; i++)
    ret_matrix._values[i] = _values[i] + rhs._values[i];

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator-(const ColumnMajorMatrixTempl<T> & rhs) const
{
  this->checkShapeEquality(rhs);

  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols);

  for (unsigned int i = 0; i < _n_entries; i++)
    ret_matrix._values[i] = _values[i] - rhs._values[i];

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator+=(const ColumnMajorMatrixTempl<T> & rhs)
{
  this->checkShapeEquality(rhs);

  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] += rhs._values[i];

  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator+=(const TypeTensor<T> & rhs)
{
  if ((_n_rows != Moose::dim) || (_n_cols != Moose::dim))
    mooseError("Cannot perform matrix addition and assignment! The shapes of the two operands are "
               "not compatible!");

  for (const auto j : libMesh::make_range(Moose::dim))
    for (const auto i : libMesh::make_range(Moose::dim))
      (*this)(i, j) += rhs(i, j);

  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator-=(const ColumnMajorMatrixTempl<T> & rhs)
{
  this->checkShapeEquality(rhs);

  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] -= rhs._values[i];

  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T>
ColumnMajorMatrixTempl<T>::operator+(T scalar) const
{
  ColumnMajorMatrixTempl<T> ret_matrix(_n_rows, _n_cols);

  for (unsigned int i = 0; i < _n_entries; i++)
    ret_matrix._values[i] = _values[i] + scalar;

  return ret_matrix;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator*=(T scalar)
{
  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] *= scalar;
  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator/=(T scalar)
{
  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] /= scalar;
  return *this;
}

template <typename T>
inline ColumnMajorMatrixTempl<T> &
ColumnMajorMatrixTempl<T>::operator+=(T scalar)
{
  for (unsigned int i = 0; i < _n_entries; i++)
    _values[i] += scalar;
  return *this;
}

template <typename T>
inline bool
ColumnMajorMatrixTempl<T>::operator==(const ColumnMajorMatrixTempl<T> & rhs) const
{
  if (_n_entries != rhs._n_entries || _n_rows != rhs._n_rows || _n_cols != rhs._n_cols)
    return false;
  return std::equal(_values.begin(), _values.end(), rhs._values.begin());
}

template <typename T>
inline bool
ColumnMajorMatrixTempl<T>::operator!=(const ColumnMajorMatrixTempl<T> & rhs) const
{
  return !(*this == rhs);
}

typedef ColumnMajorMatrixTempl<Real> ColumnMajorMatrix;

namespace MetaPhysicL
{
template <typename T>
struct RawType<ColumnMajorMatrixTempl<T>>
{
  typedef ColumnMajorMatrixTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const ColumnMajorMatrixTempl<T> & in)
  {
    value_type ret(in.m(), in.n());
    for (MooseIndex(in.m()) i = 0; i < in.m(); ++i)
      for (MooseIndex(in.n()) j = 0; j < in.n(); ++j)
        ret(i, j) = raw_value(in(i, j));

    return ret;
  }
};
}
