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
 * The SOU limiter is used for reproducing the second-order-upwind scheme. The limiter function
 * $\beta(delta_max)$ is defined as:
 *
 * \f[
 * \beta(delta_max) = min(1, delta_face/delta_max)
 * \f]
 *
 * where:
 * \( delta_max \) is the maximum variation admited for the computatioanl stencil
 * \( delta_max \) is the variation at the face computed with SOU
 *
 * @tparam T The data type of the scalar values and the return type.
 */
template <typename T>
class SOULimiter : public Limiter<T>
{
public:
  /**
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the SOU limiter formula.
   *
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @param max_value The maximum value in the current element.
   * @param min_value The minimum value in the current element.
   * @param fi Pointer to the face information structure.
   * @param fi_elem_is_upwind Boolean flag indicating if the current element is upwind.
   * @return The computed flux limiting ratio.
   */
  T limit(const T & /* phi_upwind */,
          const T & /* phi_downwind */,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * /* grad_phi_downwind */,
          const RealVectorValue & dCD,
          const Real & max_value,
          const Real & min_value,
          const FaceInfo * /* fi */,
          const bool & /*fi_elem_is_upwind */) const override final
  {
    mooseAssert(grad_phi_upwind, "SOU limiter requires a gradient");

    T delta_face = std::abs((*grad_phi_upwind) * dCD);
    T delta_max = std::abs(max_value - min_value);

    if (delta_face > 1e-10)
      return std::min(1.0, delta_max / delta_face);
    else
      return T(1.0);
  }

  bool constant() const override final { return false; }

  InterpMethod interpMethod() const override final { return InterpMethod::SOU; }

  SOULimiter() = default;
};
}
}
