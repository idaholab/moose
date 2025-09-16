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
#include "KokkosArray.h"

#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/tensor_tools.h"

namespace Moose
{
namespace Kokkos
{

struct Real33
{
  Real a[3][3];

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION Real33() { *this = 0; }
  KOKKOS_INLINE_FUNCTION Real33(const Real & scalar) { *this = scalar; }
  KOKKOS_INLINE_FUNCTION Real33(const Real33 & tensor) { *this = tensor; }
  KOKKOS_INLINE_FUNCTION Real & operator()(unsigned int i, unsigned int j) { return a[i][j]; }
  KOKKOS_INLINE_FUNCTION Real operator()(unsigned int i, unsigned int j) const { return a[i][j]; }
  KOKKOS_INLINE_FUNCTION Real33 & operator=(const Real33 & tensor)
  {
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        a[i][j] = tensor.a[i][j];

    return *this;
  }
  KOKKOS_INLINE_FUNCTION Real33 & operator=(const Real scalar)
  {
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        a[i][j] = scalar;

    return *this;
  }
  KOKKOS_INLINE_FUNCTION void operator+=(const Real33 tensor)
  {
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        a[i][j] += tensor.a[i][j];
  }
  KOKKOS_INLINE_FUNCTION Real determinant(const unsigned int dim = 3)
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
  KOKKOS_INLINE_FUNCTION Real33 inverse(const unsigned int dim = 3)
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
  KOKKOS_INLINE_FUNCTION Real33 transpose()
  {
    Real33 tr_mat;

    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        tr_mat(i, j) = a[j][i];

    return tr_mat;
  }
#endif
};

struct Real3
{
  Real v[3];

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION Real3()
  {
    v[0] = 0;
    v[1] = 0;
    v[2] = 0;
  }
  KOKKOS_INLINE_FUNCTION Real3(const Real & scalar)
  {
    v[0] = scalar;
    v[1] = scalar;
    v[2] = scalar;
  }
  KOKKOS_INLINE_FUNCTION Real3(const Real3 & vector)
  {
    v[0] = vector.v[0];
    v[1] = vector.v[1];
    v[2] = vector.v[2];
  }
  KOKKOS_INLINE_FUNCTION Real3(const Real & x, const Real & y, const Real & z)
  {
    v[0] = x;
    v[1] = y;
    v[2] = z;
  }
  Real3(const libMesh::TypeVector<Real> & vector)
  {
    v[0] = vector(0);
    v[1] = vector(1);
    v[2] = vector(2);
  }

  KOKKOS_INLINE_FUNCTION Real & operator()(unsigned int i) { return v[i]; }
  KOKKOS_INLINE_FUNCTION Real operator()(unsigned int i) const { return v[i]; }

  KOKKOS_INLINE_FUNCTION Real3 & operator=(const Real3 & vector)
  {
    v[0] = vector.v[0];
    v[1] = vector.v[1];
    v[2] = vector.v[2];

    return *this;
  }
  KOKKOS_INLINE_FUNCTION Real3 & operator=(const Real scalar)
  {
    v[0] = scalar;
    v[1] = scalar;
    v[2] = scalar;

    return *this;
  }
  Real3 & operator=(const libMesh::TypeVector<Real> & vector)
  {
    v[0] = vector(0);
    v[1] = vector(1);
    v[2] = vector(2);

    return *this;
  }
  KOKKOS_INLINE_FUNCTION void operator+=(const Real scalar)
  {
    v[0] += scalar;
    v[1] += scalar;
    v[2] += scalar;
  }
  KOKKOS_INLINE_FUNCTION void operator+=(const Real3 vector)
  {
    v[0] += vector.v[0];
    v[1] += vector.v[1];
    v[2] += vector.v[2];
  }
  KOKKOS_INLINE_FUNCTION void operator-=(const Real scalar)
  {
    v[0] -= scalar;
    v[1] -= scalar;
    v[2] -= scalar;
  }
  KOKKOS_INLINE_FUNCTION void operator-=(const Real3 vector)
  {
    v[0] -= vector.v[0];
    v[1] -= vector.v[1];
    v[2] -= vector.v[2];
  }
  KOKKOS_INLINE_FUNCTION void operator*=(const Real scalar)
  {
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
  }
  KOKKOS_INLINE_FUNCTION Real norm() { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
  KOKKOS_INLINE_FUNCTION Real dot_product(const Real3 vector)
  {
    return v[0] * vector.v[0] + v[1] * vector.v[1] + v[2] * vector.v[2];
  }
  KOKKOS_INLINE_FUNCTION Real3 cross_product(const Real3 vector)
  {
    Real3 cross;

    cross.v[0] = v[1] * vector.v[2] - v[2] * vector.v[1];
    cross.v[1] = v[2] * vector.v[0] - v[0] * vector.v[2];
    cross.v[2] = v[0] * vector.v[1] - v[1] * vector.v[0];

    return cross;
  }
  KOKKOS_INLINE_FUNCTION Real33 cartesian_product(const Real3 vector)
  {
    Real33 tensor;

    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        tensor(i, j) = v[i] * vector.v[j];

    return tensor;
  }
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE
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
KOKKOS_INLINE_FUNCTION Real
operator*(const Real3 left, const Real3 right)
{
  return left.v[0] * right.v[0] + left.v[1] * right.v[1] + left.v[2] * right.v[2];
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
operator+(const Real3 left, const Real3 right)
{
  return {left.v[0] + right.v[0], left.v[1] + right.v[1], left.v[2] + right.v[2]};
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
operator-(const Real3 left, const Real3 right)
{
  return {left.v[0] - right.v[0], left.v[1] - right.v[1], left.v[2] - right.v[2]};
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

} // namespace Kokkos
} // namespace Moose
