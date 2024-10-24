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
 * @brief Implements the Second-Order Upwind (SOU) limiter for flux limiting in numerical methods.
 *
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
   * @brief Computes the limited value using the SOU limiter.
   *
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
   *
   * This method performs the following steps:
   * 1. Asserts that the upwind gradient pointer is not null.
   * 2. Handles initialization conflict by determining the face centroid and the appropriate cell
   * centroid.
   * 3. Computes the absolute delta value at the face.
   * 4. Computes the delta between the two elements.
   * 5. Returns the limited value based on the computed deltas.
   *
   * @example
   * @code
   * SOULimiter<Real> sou;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real max_value = 1.0;
   * Real min_value = 0.0;
   * FaceInfo fi;
   * bool fi_elem_is_upwind = true;
   * Real result = sou.limit(0.0, 0.0, &grad_upwind, nullptr, dCD, max_value, min_value, &fi,
   * fi_elem_is_upwind);
   * @endcode
   */
  T limit(const T & phi_upwind,
          const T & phi_downwind,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * grad_phi_downwind,
          const RealVectorValue & dCD,
          const T & max_value,
          const T & min_value,
          const FaceInfo * fi,
          const bool & fi_elem_is_upwind) const override final
  {
    mooseAssert(grad_phi_upwind, "SOU limiter requires a gradient");

    T limiter;
    if (!grad_phi_downwind)
    {
      // Determine the face centroid and the appropriate cell centroid
      const auto & face_centroid = fi->faceCentroid();
      const auto & cell_centroid = fi_elem_is_upwind ? fi->elemCentroid() : fi->neighborCentroid();

      // Compute the abs delta value at the face
      const auto & delta_face =
          std::abs((*grad_phi_upwind) * (face_centroid - cell_centroid)) + 1e-10;

      // Compute the delta between the two elements
      const auto & elem_delta = max_value - min_value;

      // Return the limited value
      if (elem_delta > 1e-10)
        limiter = std::min(1.0, elem_delta / delta_face);
      else
        limiter = T(1.0);
    }
    else
    {
      const auto & r_f = Moose::FV::rF(phi_upwind, phi_downwind, *grad_phi_upwind, dCD);
      limiter = r_f;
    }
    return limiter;
  }

  /**
   * @brief Indicates whether the SOU limiter is constant.
   *
   * This method always returns `false` as the SOU limiter is not a constant limiter.
   *
   * @return `false` indicating the SOU limiter is not constant.
   */
  bool constant() const override final { return false; }

  /**
   * @brief Returns the interpolation method used by the SOU limiter.
   *
   * This method returns `InterpMethod::SOU`, indicating the interpolation method used.
   *
   * @return The interpolation method `InterpMethod::SOU`.
   */
  InterpMethod interpMethod() const override final { return InterpMethod::SOU; }

  /**
   * @brief Default constructor for the SOU limiter.
   */
  SOULimiter() = default;
};
}
}
