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
 * The Venkatakrishnan limiter is derived from the following equations:
 *
 * 1. Calculation of the face delta:
 * \f[
 * \Delta_{\text{face}} = \nabla \phi_{\text{upwind}} \cdot (\mathbf{x}_{\text{face}} -
 * \mathbf{x}_{\text{cell}}) \f]
 *
 * 2. Calculation of the deltas for maximum and minimum values relative to the upwind value with a
 * small perturbation to avoid division by zero: \f[ \Delta_{\text{max}} = \phi_{\text{max}} -
 * \phi_{\text{upwind}} + 1e-10 \f] \f[ \Delta_{\text{min}} = \phi_{\text{min}} -
 * \phi_{\text{upwind}} + 1e-10 \f]
 *
 * 3. Calculation of the gradient ratio coefficient \( r_f \):
 * \f[
 * r_f = \begin{cases}
 * \frac{\Delta_{\text{face}}}{\Delta_{\text{max}}} & \text{if } \Delta_{\text{face}} \geq 0 \\
 * \frac{\Delta_{\text{face}}}{\Delta_{\text{min}}} & \text{otherwise}
 * \end{cases}
 * \f]
 *
 * 4. Venkatakrishnan limiter formula (Venkatakrishnan, 1993):
 * \f[
 * \beta(r_f) = \frac{2 r_f + 1.0}{r_f (2 r_f + 1.0) + 1.0}
 * \f]
 *
 * @tparam T The data type of the scalar values and the return type.
 */
template <typename T>
class VenkatakrishnanLimiter : public Limiter<T>
{
public:
  /**
   * This method overrides the pure virtual `limit` method in the base `Limiter` class.
   * It calculates the flux limiting ratio based on the Venkatakrishnan limiter formula.
   *
   * @param phi_upwind Scalar value at the upwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param max_value The maximum value in the current element.
   * @param min_value The minimum value in the current element.
   * @param fi Pointer to the face information structure.
   * @param fi_elem_is_upwind Boolean flag indicating if the current element is upwind.
   * @return The computed flux limiting ratio.
   */
  T limit(const T & phi_upwind,
          const T & /* phi_downwind */,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * /* grad_phi_downwind */,
          const RealVectorValue & /* dCD */,
          const Real & max_value,
          const Real & min_value,
          const FaceInfo * fi,
          const bool & fi_elem_is_upwind) const override final
  {
    const auto face_centroid = fi->faceCentroid();
    const auto cell_centroid = fi_elem_is_upwind ? fi->elemCentroid() : fi->neighborCentroid();

    const auto delta_face = (*grad_phi_upwind) * (face_centroid - cell_centroid);
    const auto delta_max = std::abs(max_value - phi_upwind) + 1e-10;
    const auto delta_min = std::abs(min_value - phi_upwind) + 1e-10;

    const auto rf =
        delta_face >= 0 ? std::abs(delta_face) / delta_max : std::abs(delta_face) / delta_min;

    return (2 * rf + 1.0) / (rf * (2 * rf + 1.0) + 1.0);
  }

  bool constant() const override final { return false; }

  InterpMethod interpMethod() const override final { return InterpMethod::Venkatakrishnan; }

  VenkatakrishnanLimiter() = default;
};
}
}
