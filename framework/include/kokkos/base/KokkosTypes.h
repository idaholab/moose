//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

namespace Moose::Kokkos
{

using Real3 = libMesh::VectorValue<Real>;
using ADReal3 = libMesh::VectorValue<ADReal>;

template <typename T>
struct Tensor33;

using Real33 = Tensor33<Real>;

/**
 * 3x3 tensor usable on device. Derives from libMesh::TypeTensor so that the libMesh tensor
 * algebra is shared, while construction from a scalar broadcasts it to every component (libMesh
 * only accepts zero there) and the identity, determinant, and inverse act on the leading dim x dim
 * block.
 */
template <typename T>
struct Tensor33 : public libMesh::TypeTensor<T>
{
#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION Tensor33() = default;
  KOKKOS_INLINE_FUNCTION Tensor33(const T & scalar)
    : libMesh::TypeTensor<T>(scalar, scalar, scalar, scalar, scalar, scalar, scalar, scalar, scalar)
  {
  }
  /// Implicit conversion so that results of the libMesh::TypeTensor operators are Tensor33 again
  template <typename U>
  KOKKOS_INLINE_FUNCTION Tensor33(const libMesh::TypeTensor<U> & tensor)
    : libMesh::TypeTensor<T>(tensor)
  {
  }

  /// Set the leading dim x dim block to the identity and the remaining entries to zero
  KOKKOS_INLINE_FUNCTION void identity(const unsigned int dim = 3);
  /// Determinant of the leading dim x dim block; only for arithmetic element types
  template <typename U = T, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  KOKKOS_INLINE_FUNCTION T determinant(const unsigned int dim = 3) const;
  /// Inverse of the leading dim x dim block, with the remaining entries zero; only for arithmetic
  /// element types
  template <typename U = T, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  KOKKOS_INLINE_FUNCTION Tensor33<T> inverse(const unsigned int dim = 3) const;
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T>
KOKKOS_INLINE_FUNCTION void
Tensor33<T>::identity(const unsigned int dim)
{
  this->zero();

  for (unsigned int i = 0; i < dim; ++i)
    (*this)(i, i) = 1;
}

template <typename T>
template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int>>
KOKKOS_INLINE_FUNCTION T
Tensor33<T>::determinant(const unsigned int dim) const
{
  const auto & a = *this;
  T det = 0;

  if (dim == 0)
    det = 1;
  else if (dim == 1)
    det = a(0, 0);
  else if (dim == 2)
    det = a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0);
  else if (dim == 3)
    det = a(0, 0) * (a(1, 1) * a(2, 2) - a(1, 2) * a(2, 1)) -
          a(0, 1) * (a(1, 0) * a(2, 2) - a(1, 2) * a(2, 0)) +
          a(0, 2) * (a(1, 0) * a(2, 1) - a(1, 1) * a(2, 0));

  return det;
}

template <typename T>
template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int>>
KOKKOS_INLINE_FUNCTION Tensor33<T>
Tensor33<T>::inverse(const unsigned int dim) const
{
  const auto & a = *this;
  const T inv_det = 1.0 / determinant(dim);
  Tensor33<T> inv_mat;

  if (dim == 1)
  {
    inv_mat(0, 0) = inv_det;
  }
  else if (dim == 2)
  {
    inv_mat(0, 0) = a(1, 1) * inv_det;
    inv_mat(0, 1) = -a(0, 1) * inv_det;
    inv_mat(1, 0) = -a(1, 0) * inv_det;
    inv_mat(1, 1) = a(0, 0) * inv_det;
  }
  else if (dim == 3)
  {
    inv_mat(0, 0) = (a(1, 1) * a(2, 2) - a(1, 2) * a(2, 1)) * inv_det;
    inv_mat(0, 1) = (a(0, 2) * a(2, 1) - a(0, 1) * a(2, 2)) * inv_det;
    inv_mat(0, 2) = (a(0, 1) * a(1, 2) - a(0, 2) * a(1, 1)) * inv_det;
    inv_mat(1, 0) = (a(1, 2) * a(2, 0) - a(1, 0) * a(2, 2)) * inv_det;
    inv_mat(1, 1) = (a(0, 0) * a(2, 2) - a(0, 2) * a(2, 0)) * inv_det;
    inv_mat(1, 2) = (a(0, 2) * a(1, 0) - a(0, 0) * a(1, 2)) * inv_det;
    inv_mat(2, 0) = (a(1, 0) * a(2, 1) - a(1, 1) * a(2, 0)) * inv_det;
    inv_mat(2, 1) = (a(0, 1) * a(2, 0) - a(0, 0) * a(2, 1)) * inv_det;
    inv_mat(2, 2) = (a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0)) * inv_det;
  }

  return inv_mat;
}

KOKKOS_INLINE_FUNCTION Real3
curlFromVectorGradient(const Real33 grad, const unsigned int dim)
{
  if (dim < 2)
    return 0;
  else if (dim == 2)
    return Real3(0, 0, grad(1, 0) - grad(0, 1));
  else
    return Real3(grad(2, 1) - grad(1, 2), grad(0, 2) - grad(2, 0), grad(1, 0) - grad(0, 1));
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
