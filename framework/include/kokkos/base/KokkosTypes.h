//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosThread.h"
#include "KokkosScalar.h"
#include "KokkosJaggedArray.h"

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosADReal.h"
#endif

#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/tensor_tools.h"

namespace Moose::Kokkos
{

template <typename T>
struct Vector3;

using Real3 = Vector3<Real>;
using ADReal3 = Vector3<ADReal>;

struct Real33;

template <typename T>
struct Vector3
{
  T v[3];

#ifdef MOOSE_KOKKOS_SCOPE
  Vector3(const libMesh::TypeVector<T> & vector);
  KOKKOS_INLINE_FUNCTION Vector3() { *this = T(0); }
  KOKKOS_INLINE_FUNCTION Vector3(const T & scalar) { *this = scalar; }
  KOKKOS_INLINE_FUNCTION Vector3(const Vector3<T> & vector) { *this = vector; }
  KOKKOS_INLINE_FUNCTION Vector3(const T & x, const T & y, const T & z);

  KOKKOS_INLINE_FUNCTION Vector3<T> operator-() const;
  KOKKOS_INLINE_FUNCTION T & operator()(unsigned int i) { return v[i]; }
  KOKKOS_INLINE_FUNCTION const T & operator()(unsigned int i) const { return v[i]; }

  Vector3<T> & operator=(const libMesh::TypeVector<T> & vector);

  template <typename U>
  KOKKOS_INLINE_FUNCTION Vector3<T> & operator=(const Vector3<U> & vector);
  KOKKOS_INLINE_FUNCTION Vector3<T> & operator=(const Vector3<T> & vector);
  KOKKOS_INLINE_FUNCTION Vector3<T> & operator=(const T & scalar);

  template <typename U>
  KOKKOS_INLINE_FUNCTION void operator+=(const Vector3<U> & vector);
  KOKKOS_INLINE_FUNCTION void operator+=(const T & scalar);
  template <typename U>
  KOKKOS_INLINE_FUNCTION void operator-=(const Vector3<U> & vector);
  KOKKOS_INLINE_FUNCTION void operator-=(const T & scalar);
  KOKKOS_INLINE_FUNCTION void operator*=(const T & scalar);

  KOKKOS_INLINE_FUNCTION Real norm() const;
  KOKKOS_INLINE_FUNCTION Real dot_product(const Real3 vector) const;
  KOKKOS_INLINE_FUNCTION Real3 cross_product(const Real3 vector) const;
  KOKKOS_INLINE_FUNCTION Real33 cartesian_product(const Real3 vector) const;
#endif
};

struct Real33
{
  Real a[3][3];

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION Real33() { *this = 0; }
  KOKKOS_INLINE_FUNCTION Real33(const Real scalar) { *this = scalar; }
  KOKKOS_INLINE_FUNCTION Real33(const Real33 & tensor) { *this = tensor; }

  KOKKOS_INLINE_FUNCTION Real & operator()(unsigned int i, unsigned int j) { return a[i][j]; }
  KOKKOS_INLINE_FUNCTION Real operator()(unsigned int i, unsigned int j) const { return a[i][j]; }

  KOKKOS_INLINE_FUNCTION Real33 & operator=(const Real33 & tensor);
  KOKKOS_INLINE_FUNCTION Real33 & operator=(const Real scalar);
  KOKKOS_INLINE_FUNCTION void operator+=(const Real33 tensor);

  KOKKOS_INLINE_FUNCTION void identity(const unsigned int dim = 3);
  KOKKOS_INLINE_FUNCTION Real determinant(const unsigned int dim = 3) const;
  KOKKOS_INLINE_FUNCTION Real33 inverse(const unsigned int dim = 3) const;
  KOKKOS_INLINE_FUNCTION Real33 transpose() const;
  KOKKOS_INLINE_FUNCTION Real3 row(const unsigned int i) const;
  KOKKOS_INLINE_FUNCTION Real3 col(const unsigned int j) const;
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE

template <typename T>
Vector3<T>::Vector3(const libMesh::TypeVector<T> & vector)
{
  v[0] = vector(0);
  v[1] = vector(1);
  v[2] = vector(2);
}

template <typename T>
KOKKOS_INLINE_FUNCTION
Vector3<T>::Vector3(const T & x, const T & y, const T & z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

template <typename T>
Vector3<T> &
Vector3<T>::operator=(const libMesh::TypeVector<T> & vector)
{
  v[0] = vector(0);
  v[1] = vector(1);
  v[2] = vector(2);

  return *this;
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
Vector3<T>::operator-() const
{
  Vector3<T> vector(*this);
  vector *= -1;

  return vector;
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T> &
Vector3<T>::operator=(const Vector3<T> & vector)
{
  v[0] = vector.v[0];
  v[1] = vector.v[1];
  v[2] = vector.v[2];

  return *this;
}

template <typename T>
template <typename U>
KOKKOS_INLINE_FUNCTION Vector3<T> &
Vector3<T>::operator=(const Vector3<U> & vector)
{
  v[0] = vector.v[0];
  v[1] = vector.v[1];
  v[2] = vector.v[2];

  return *this;
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T> &
Vector3<T>::operator=(const T & scalar)
{
  v[0] = scalar;
  v[1] = scalar;
  v[2] = scalar;

  return *this;
}

template <typename T>
template <typename U>
KOKKOS_INLINE_FUNCTION void
Vector3<T>::operator+=(const Vector3<U> & vector)
{
  v[0] += vector.v[0];
  v[1] += vector.v[1];
  v[2] += vector.v[2];
}

template <typename T>
KOKKOS_INLINE_FUNCTION void
Vector3<T>::operator+=(const T & scalar)
{
  v[0] += scalar;
  v[1] += scalar;
  v[2] += scalar;
}

template <typename T>
template <typename U>
KOKKOS_INLINE_FUNCTION void
Vector3<T>::operator-=(const Vector3<U> & vector)
{
  v[0] -= vector.v[0];
  v[1] -= vector.v[1];
  v[2] -= vector.v[2];
}

template <typename T>
KOKKOS_INLINE_FUNCTION void
Vector3<T>::operator-=(const T & scalar)
{
  v[0] -= scalar;
  v[1] -= scalar;
  v[2] -= scalar;
}

template <typename T>
KOKKOS_INLINE_FUNCTION void
Vector3<T>::operator*=(const T & scalar)
{
  v[0] *= scalar;
  v[1] *= scalar;
  v[2] *= scalar;
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator+(const T & left, const Vector3<T> & right)
{
  return {left + right.v[0], left + right.v[1], left + right.v[2]};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator+(const Vector3<T> & left, const T & right)
{
  return {left.v[0] + right, left.v[1] + right, left.v[2] + right};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator+(const Vector3<T> & left, const Vector3<T> & right)
{
  return {left.v[0] + right.v[0], left.v[1] + right.v[1], left.v[2] + right.v[2]};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator-(const T & left, const Vector3<T> & right)
{
  return {left - right.v[0], left - right.v[1], left - right.v[2]};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator-(const Vector3<T> & left, const T & right)
{
  return {left.v[0] - right, left.v[1] - right, left.v[2] - right};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator-(const Vector3<T> & left, const Vector3<T> & right)
{
  return {left.v[0] - right.v[0], left.v[1] - right.v[1], left.v[2] - right.v[2]};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator*(const T & left, const Vector3<T> & right)
{
  return {left * right.v[0], left * right.v[1], left * right.v[2]};
}

template <typename T>
KOKKOS_INLINE_FUNCTION Vector3<T>
operator*(const Vector3<T> & left, const T & right)
{
  return {left.v[0] * right, left.v[1] * right, left.v[2] * right};
}

template <typename T>
KOKKOS_INLINE_FUNCTION T
operator*(const Vector3<T> & left, const Vector3<T> & right)
{
  return left.v[0] * right.v[0] + left.v[1] * right.v[1] + left.v[2] * right.v[2];
}

template <>
KOKKOS_INLINE_FUNCTION Real
Vector3<Real>::norm() const
{
  return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

template <>
KOKKOS_INLINE_FUNCTION Real
Vector3<Real>::dot_product(const Real3 vector) const
{
  return v[0] * vector.v[0] + v[1] * vector.v[1] + v[2] * vector.v[2];
}

template <>
KOKKOS_INLINE_FUNCTION Real3
Vector3<Real>::cross_product(const Real3 vector) const
{
  Real3 cross;

  cross.v[0] = v[1] * vector.v[2] - v[2] * vector.v[1];
  cross.v[1] = v[2] * vector.v[0] - v[0] * vector.v[2];
  cross.v[2] = v[0] * vector.v[1] - v[1] * vector.v[0];

  return cross;
}

template <>
KOKKOS_INLINE_FUNCTION Real33
Vector3<Real>::cartesian_product(const Real3 vector) const
{
  Real33 tensor;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      tensor(i, j) = v[i] * vector.v[j];

  return tensor;
}

KOKKOS_INLINE_FUNCTION Real33 &
Real33::operator=(const Real33 & tensor)
{
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      a[i][j] = tensor.a[i][j];

  return *this;
}

KOKKOS_INLINE_FUNCTION Real33 &
Real33::operator=(const Real scalar)
{
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      a[i][j] = scalar;

  return *this;
}

KOKKOS_INLINE_FUNCTION void
Real33::operator+=(const Real33 tensor)
{
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      a[i][j] += tensor.a[i][j];
}

KOKKOS_INLINE_FUNCTION void
Real33::identity(const unsigned int dim)
{
  *this = 0;

  for (unsigned int i = 0; i < dim; ++i)
    a[i][i] = 1;
}

KOKKOS_INLINE_FUNCTION Real
Real33::determinant(const unsigned int dim) const
{
  Real det = 0;

  if (dim == 0)
    det = 1;
  else if (dim == 1)
    det = a[0][0];
  else if (dim == 2)
    det = a[0][0] * a[1][1] - a[0][1] * a[1][0];
  else if (dim == 3)
    det = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
          a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
          a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);

  return det;
}

KOKKOS_INLINE_FUNCTION Real33
Real33::inverse(const unsigned int dim) const
{
  Real inv_det = 1.0 / determinant(dim);
  Real33 inv_mat;

  if (dim == 1)
  {
    inv_mat(0, 0) = inv_det;
  }
  else if (dim == 2)
  {
    inv_mat(0, 0) = a[1][1] * inv_det;
    inv_mat(0, 1) = -a[0][1] * inv_det;
    inv_mat(1, 0) = -a[1][0] * inv_det;
    inv_mat(1, 1) = a[0][0] * inv_det;
  }
  else if (dim == 3)
  {
    inv_mat(0, 0) = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) * inv_det;
    inv_mat(0, 1) = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) * inv_det;
    inv_mat(0, 2) = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) * inv_det;
    inv_mat(1, 0) = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) * inv_det;
    inv_mat(1, 1) = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) * inv_det;
    inv_mat(1, 2) = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) * inv_det;
    inv_mat(2, 0) = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) * inv_det;
    inv_mat(2, 1) = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) * inv_det;
    inv_mat(2, 2) = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) * inv_det;
  }

  return inv_mat;
}

KOKKOS_INLINE_FUNCTION Real33
Real33::transpose() const
{
  Real33 tr_mat;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      tr_mat(i, j) = a[j][i];

  return tr_mat;
}

KOKKOS_INLINE_FUNCTION Real3
Real33::row(const unsigned int i) const
{
  return Real3(a[i][0], a[i][1], a[i][2]);
}

KOKKOS_INLINE_FUNCTION Real3
Real33::col(const unsigned int j) const
{
  return Real3(a[0][j], a[1][j], a[2][j]);
}

KOKKOS_INLINE_FUNCTION Real3
operator*(const Real33 left, const Real3 right)
{
  return {left(0, 0) * right.v[0] + left(0, 1) * right.v[1] + left(0, 2) * right.v[2],
          left(1, 0) * right.v[0] + left(1, 1) * right.v[1] + left(1, 2) * right.v[2],
          left(2, 0) * right.v[0] + left(2, 1) * right.v[1] + left(2, 2) * right.v[2]};
}

KOKKOS_INLINE_FUNCTION Real33
operator*(const Real33 left, const Real33 right)
{
  Real33 mul;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        mul(i, j) += left(i, k) * right(k, j);

  return mul;
}

KOKKOS_INLINE_FUNCTION Real3
operator+(const Real left, const Real3 right)
{
  return {left + right.v[0], left + right.v[1], left + right.v[2]};
}

KOKKOS_INLINE_FUNCTION Real3
operator+(const Real3 left, const Real right)
{
  return {left.v[0] + right, left.v[1] + right, left.v[2] + right};
}

KOKKOS_INLINE_FUNCTION Real3
operator-(const Real left, const Real3 right)
{
  return {left - right.v[0], left - right.v[1], left - right.v[2]};
}

KOKKOS_INLINE_FUNCTION Real3
operator-(const Real3 left, const Real right)
{
  return {left.v[0] - right, left.v[1] - right, left.v[2] - right};
}

KOKKOS_INLINE_FUNCTION Real3
operator*(const Real left, const Real3 right)
{
  return {left * right.v[0], left * right.v[1], left * right.v[2]};
}

KOKKOS_INLINE_FUNCTION Real3
operator*(const Real3 left, const Real right)
{
  return {left.v[0] * right, left.v[1] * right, left.v[2] * right};
}

template <typename T,
          typename = typename std::enable_if<
              std::is_same<typename std::decay<T>::type, ADReal>::value>::type>
KOKKOS_INLINE_FUNCTION ADReal3
operator*(const Real3 left, const T & right)
{
  return {left(0) * right, left(1) * right, left(2) * right};
}

template <typename T,
          typename = typename std::enable_if<
              std::is_same<typename std::decay<T>::type, ADReal>::value>::type>
KOKKOS_INLINE_FUNCTION ADReal3
operator*(const T & left, const Real3 right)
{
  return {left * right(0), left * right(1), left * right(2)};
}

KOKKOS_INLINE_FUNCTION ADReal
operator*(const Real3 left, const ADReal3 & right)
{
  return left(0) * right(0) + left(1) * right(1) + left(2) * right(2);
}

KOKKOS_INLINE_FUNCTION ADReal
operator*(const ADReal3 & left, const Real3 right)
{
  return left(0) * right(0) + left(1) * right(1) + left(2) * right(2);
}

#endif

template <typename T1, typename T2>
struct Pair
{
  T1 first;
  T2 second;

  template <typename T3, typename T4>
  auto & operator=(const std::pair<T3, T4> pair)
  {
    first = pair.first;
    second = pair.second;

    return *this;
  }
};

template <typename T1, typename T2>
bool
operator<(const Pair<T1, T2> & left, const Pair<T1, T2> & right)
{
  return std::make_pair(left.first, left.second) < std::make_pair(right.first, right.second);
}

} // namespace Moose::Kokkos
