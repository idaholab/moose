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
 * The QUICK (Quadratic Upstream Interpolation for Convective Kinematics) limiter
 * function is derived from the following equations:
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
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the QUICK limiter formula.
   *
   * @param phi_upwind Scalar value at the upwind location.
   * @param phi_downwind Scalar value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed flux limiting ratio.
   */
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * grad_phi_downwind,
          const RealVectorValue & dCD,
          const Real & /* max_value */,
          const Real & /* min_value */,
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

  bool constant() const override final { return false; }

  InterpMethod interpMethod() const override final { return InterpMethod::QUICK; }

  QUICKLimiter() = default;
};
}
}
