//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * The Van Leer limiter limiter function $\beta(r_f)$ is defined as:
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
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the Van Leer limiter formula.
   *
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
    mooseAssert(grad_phi_upwind, "Van Leer limiter requires the upwind gradient");
    using std::abs;

    // Compute gradient ratio coefficient
    T r_f;
    if (grad_phi_downwind) // compute full slope-reconstruction limiter
      r_f = this->rf_grad(grad_phi_upwind, grad_phi_downwind, dCD);
    else // compute upwind slope reconstruction limiter
      r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);

    // Return limiter value
    return (r_f + abs(r_f)) / (1.0 + abs(r_f));
  }

  bool constant() const override final { return false; }

  InterpMethod interpMethod() const override final { return InterpMethod::VanLeer; }

  VanLeerLimiter() = default;
};
}
}
