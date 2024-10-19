//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Limiter.h"
#include "MathFVUtils.h"

namespace Moose
{
namespace FV
{
/**
 * @brief Implements the Van Leer limiter for flux limiting in numerical methods.
 *
 * The Van Leer limiter is a slope limiter used to reduce numerical oscillations and
 * enforce monotonicity in computational fluid dynamics (CFD) and other numerical simulations.
 * The limiter function $\beta(r_f)$ is defined as:
 *
 * \f[
 * \beta(r_f) = \frac{r_f + \text{abs}(r_f)}{1 + \text{abs}(r_f)}
 * \f]
 *
 * where \( r_f \) is the ratio of the upwind to downwind gradients.
 *
 * @tparam T The data type of the scalar values and the return type.
 */
template <typename T>
class VanLeerLimiter : public Limiter<T>
{
public:
  /**
   * @brief Computes the limited value using the Van Leer limiter.
   *
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the Van Leer limiter formula.
   *
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed flux limiting ratio.
   *
   * This method performs the following steps:
   * 1. Asserts that the upwind and downwind gradient pointers are not null.
   * 2. Computes the gradient ratio coefficient \( r_f \) using the `rf_grad` method.
   * 3. Applies the Van Leer limiter formula to \( r_f \) to obtain the limited value.
   * 4. Returns the computed limited value.
   *
   * @example
   * @code
   * VanLeerLimiter<Real> vanLeer;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * VectorValue<Real> grad_downwind(0.4, 0.5, 0.6);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real result = vanLeer.limit(0.0, 0.0, &grad_upwind, &grad_downwind, dCD, 0.0, 0.0, nullptr,
   * true);
   * @endcode
   */
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * grad_phi_downwind,
          const RealVectorValue & dCD,
          const T & /* max_value */,
          const T & /* min_value */,
          const FaceInfo * /* fi */,
          const bool & /* fi_elem_is_upwind */) const override final
  {
    mooseAssert(grad_phi_upwind, "Van Leer limiter requires the upwind gradient");

    // Compute gradient ratio coefficient
    T r_f;
    if (grad_phi_downwind) // compute full slope-reconstruction limiter
      r_f = this->rf_grad(grad_phi_upwind, grad_phi_downwind, dCD);
    else // compute upwind slope reconstruction limiter
      r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);

    // Return limiter value
    return (r_f + std::abs(r_f)) / (1.0 + std::abs(r_f));
  }

  /**
   * @brief Indicates whether the Van Leer limiter is constant.
   *
   * This method always returns `false` as the Van Leer limiter is not a constant limiter.
   *
   * @return `false` indicating the Van Leer limiter is not constant.
   */
  bool constant() const override final { return false; }

  /**
   * @brief Returns the interpolation method used by the Van Leer limiter.
   *
   * This method returns `InterpMethod::VanLeer`, indicating the interpolation method used.
   *
   * @return The interpolation method `InterpMethod::VanLeer`.
   */
  InterpMethod interpMethod() const override final { return InterpMethod::VanLeer; }

  /**
   * @brief Default constructor for the Van Leer limiter.
   */
  VanLeerLimiter() = default;
};
}
}
