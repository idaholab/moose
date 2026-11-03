//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

namespace Moose::Kokkos
{

/**
 * Number of derivative slots of the displacements of the current configuration: the value and the
 * reference gradient of each of the three components
 */
constexpr unsigned int AD_DISPLACEMENT_SLOTS = 3 * (AD_VALUE_SLOTS + AD_GRADIENT_SLOTS);

/**
 * Dense 3x3 tensor of dual numbers, which carries the derivatives of the geometry of the current
 * configuration with respect to the displacements
 * @tparam slots The number of derivative slots
 */
template <unsigned int slots>
struct DenseADReal33
{
  DenseADReal<slots> a[3][3];

  KOKKOS_INLINE_FUNCTION DenseADReal33() { *this = Real(0); }
  KOKKOS_INLINE_FUNCTION DenseADReal33(const Real33 & tensor)
  {
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        a[i][j] = tensor(i, j);
  }

  KOKKOS_INLINE_FUNCTION DenseADReal<slots> & operator()(unsigned int i, unsigned int j)
  {
    return a[i][j];
  }
  KOKKOS_INLINE_FUNCTION const DenseADReal<slots> & operator()(unsigned int i, unsigned int j) const
  {
    return a[i][j];
  }

  KOKKOS_INLINE_FUNCTION DenseADReal33 & operator=(const Real scalar)
  {
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        a[i][j] = scalar;

    return *this;
  }

  /**
   * Get the determinant of the leading block of a dimension
   */
  KOKKOS_INLINE_FUNCTION DenseADReal<slots> determinant(const unsigned int dim) const;
  /**
   * Get the inverse of the leading block of a dimension, the identity elsewhere
   */
  KOKKOS_INLINE_FUNCTION DenseADReal33 inverse(const unsigned int dim) const;
  /**
   * Get the transpose
   */
  KOKKOS_INLINE_FUNCTION DenseADReal33 transpose() const;
};

template <unsigned int slots>
KOKKOS_INLINE_FUNCTION DenseADReal<slots>
DenseADReal33<slots>::determinant(const unsigned int dim) const
{
  if (dim == 1)
    return a[0][0];
  else if (dim == 2)
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
  else
    return a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
           a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
           a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
}

template <unsigned int slots>
KOKKOS_INLINE_FUNCTION DenseADReal33<slots>
DenseADReal33<slots>::inverse(const unsigned int dim) const
{
  const DenseADReal<slots> inv_det = 1 / determinant(dim);

  DenseADReal33 inv;

  for (unsigned int i = dim; i < 3; ++i)
    inv(i, i) = 1;

  if (dim == 1)
    inv(0, 0) = inv_det;
  else if (dim == 2)
  {
    inv(0, 0) = a[1][1] * inv_det;
    inv(0, 1) = -a[0][1] * inv_det;
    inv(1, 0) = -a[1][0] * inv_det;
    inv(1, 1) = a[0][0] * inv_det;
  }
  else
  {
    inv(0, 0) = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) * inv_det;
    inv(0, 1) = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) * inv_det;
    inv(0, 2) = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) * inv_det;
    inv(1, 0) = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) * inv_det;
    inv(1, 1) = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) * inv_det;
    inv(1, 2) = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) * inv_det;
    inv(2, 0) = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) * inv_det;
    inv(2, 1) = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) * inv_det;
    inv(2, 2) = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) * inv_det;
  }

  return inv;
}

template <unsigned int slots>
KOKKOS_INLINE_FUNCTION DenseADReal33<slots>
DenseADReal33<slots>::transpose() const
{
  DenseADReal33 tr;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      tr(i, j) = a[j][i];

  return tr;
}

/**
 * Products of dual tensors with vectors
 */
///@{
template <unsigned int slots, typename T>
KOKKOS_INLINE_FUNCTION DenseADReal3<slots>
operator*(const DenseADReal33<slots> & left, const Vector3<T> & right)
{
  DenseADReal3<slots> result;

  for (unsigned int i = 0; i < 3; ++i)
  {
    result(i) = 0;

    for (unsigned int j = 0; j < 3; ++j)
      result(i) += left(i, j) * right(j);
  }

  return result;
}
///@}

/**
 * Geometry of a quadrature point of the current configuration as dual numbers, whose derivatives
 * with respect to the displacement slots give the variation of the geometry under a displacement
 * @tparam slots The number of derivative slots
 */
template <unsigned int slots>
struct ADGeometry
{
  /// Inverse of the deformation gradient
  DenseADReal33<slots> F_inv;
  /// Inverse of the Jacobian of the map from the reference element to the current configuration
  DenseADReal33<slots> dxi_dx;
  /// Transformed Jacobian weight on the current configuration
  DenseADReal<slots> JxW;
  /// Quadrature point on the current configuration
  DenseADReal3<slots> q_point;
  /// Outward unit normal on the current configuration
  DenseADReal3<slots> normal;
};

} // namespace Moose::Kokkos
