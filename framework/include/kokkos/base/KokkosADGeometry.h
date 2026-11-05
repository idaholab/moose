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
 * Geometry of a quadrature point of the current configuration in plain reals, from which the
 * derivatives of geometric quantities with respect to the displacement slots follow in closed form.
 * With the slots of the reference gradient (grad_X u_c)_d of displacement component c,
 *
 *   d(F^{-T} g)/d(grad_X u_c)_d = -(F^{-T} g)_c (F^{-T})_{:,d}
 *   d(ln JxW)/d(grad_X u_c)_d   = (F^{-T})_{cd} - n_c (n . (F^{-T})_{:,d})
 *
 * for any reference gradient g, where the normal term applies on sides only, and the slots of the
 * value u_c move the quadrature point x = X + u.
 */
struct ADGeometry
{
  /// Transpose of the inverse of the deformation gradient
  Real33 F_inv_T;
  /// Inverse of the Jacobian of the map from the reference element to the current configuration
  Real33 dxi_dx;
  /// Transformed Jacobian weight on the current configuration
  Real JxW = 0;
  /// Quadrature point on the current configuration
  Real3 q_point;
  /// Outward unit normal on the current configuration, zero on elements
  Real3 normal;
  /// Gradient of the coordinate transform factor at the quadrature point divided by the factor
  Real3 grad_log_coord;
};

} // namespace Moose::Kokkos
