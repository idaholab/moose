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
 * @brief Implements the QUICK limiter for flux limiting in numerical methods.
 *
 * The QUICK (Quadratic Upstream Interpolation for Convective Kinematics) limiter is used to reduce
 * numerical oscillations and enforce monotonicity in computational fluid dynamics (CFD) and other
 * numerical simulations. This limiter ensures Total Variation Diminishing (TVD) compliance.
 *
 * The limiter function is derived from the following equations:
 *
 * 1. Calculation of the gradient ratio coefficient \( r_f \):
 * \f[
 * r_f = \begin{cases}
 * \text{if grad\_phi\_downwind is not null, use } \text{rf\_grad}(\nabla \phi_{\text{upwind}},
 * \nabla \phi_{\text{downwind}}, \mathbf{dCD}) \\ \text{otherwise, use }
 * \text{rF}(\phi_{\text{upwind}}, \phi_{\text{downwind}}, \nabla \phi_{\text{upwind}},
 * \mathbf{dCD}) \end{cases} \f]
 *
 * 2. QUICK limiter formula ensuring TVD compliance:
 * \f[
 * \beta(r_f) = \min\left(\beta, \max\left(\min\left(\min\left(\frac{1 + 3.0 r_f}{4.0}, 2.0
 * r_f\right), 2.0\right), 0.0\right)\right) \f] where \(\beta = 1.0\).
 *
 * @tparam T The data type of the scalar values and the return type.
 */
template <typename T>
class QUICKLimiter : public Limiter<T>
{
public:
  /**
   * @brief Computes the limited value using the QUICK limiter.
   *
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the QUICK limiter formula.
   *
   * @param phi_upwind Scalar value at the upwind location.
   * @param phi_downwind Scalar value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed flux limiting ratio.
   *
   * This method performs the following steps:
   * 1. Asserts that the upwind gradient pointer is not null.
   * 2. Computes the gradient ratio coefficient \( r_f \) using the `rf_grad` method or `rF` method.
   * 3. Applies the QUICK limiter formula to \( r_f \) to obtain the limited value, ensuring TVD
   * compliance.
   * 4. Returns the computed limited value.
   *
   * @example
   * @code
   * QUICKLimiter<Real> quick;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * VectorValue<Real> grad_downwind(0.4, 0.5, 0.6);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real result = quick.limit(0.0, 0.0, &grad_upwind, &grad_downwind, dCD, 0.0, 0.0, nullptr,
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
    mooseAssert(grad_phi_upwind, "QUICK limiter requires a gradient");

    // Compute gradient ratio coefficient
    T limiter;
    if (grad_phi_downwind) // compute full slope-reconstruction limiter for weakly-compressible flow
    {
      const auto & r_f = this->rf_grad(grad_phi_upwind, grad_phi_downwind, dCD);
      const auto & beta = T(1.0); // Ensures TVD compliance
      limiter =
          0 * r_f +
          std::min(beta,
                   std::max(std::min(std::min((1 + 3.0 * r_f) / 4.0, 2.0 * r_f), T(2.0)), T(0.0)));
    }
    else // compute upwind slope reconstruction limiter for compressible flow
    {
      const auto & r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);
      limiter = (3.0 * r_f) / 4.0;
    }

    // Return limiter value
    return limiter;
  }

  /**
   * @brief Indicates whether the QUICK limiter is constant.
   *
   * This method always returns `false` as the QUICK limiter is not a constant limiter.
   *
   * @return `false` indicating the QUICK limiter is not constant.
   */
  bool constant() const override final { return false; }

  /**
   * @brief Returns the interpolation method used by the QUICK limiter.
   *
   * This method returns `InterpMethod::QUICK`, indicating the interpolation method used.
   *
   * @return The interpolation method `InterpMethod::QUICK`.
   */
  InterpMethod interpMethod() const override final { return InterpMethod::QUICK; }

  /**
   * @brief Default constructor for the QUICK limiter.
   */
  QUICKLimiter() = default;
};
}
}
