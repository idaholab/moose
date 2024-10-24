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
 * @brief Implements the Venkatakrishnan limiter for flux limiting.
 *
 * The Venkatakrishnan limiter is used to reduce numerical oscillations and enforce monotonicity
 * in computational fluid dynamics (CFD) and other numerical simulations. This limiter adjusts
 * the flux limiting ratio based on face centroids and cell centroids, handling different gradient
 * conditions.
 *
 * The limiter function is derived from the following equations:
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
 * 4. Venkatakrishnan limiter formula:
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
   * @brief Computes the limited value using the Venkatakrishnan limiter.
   *
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
   *
   * This method performs the following steps:
   * 1. Determines the face centroid and the appropriate cell centroid.
   * 2. Computes the delta value at the face.
   * 3. Computes deltas for the maximum and minimum values relative to the upwind value.
   * 4. Computes the ratio \( r_f \) based on the sign of the delta face value.
   * 5. Applies the Venkatakrishnan limiter formula to \( r_f \) to obtain the limited value.
   * 6. Returns the computed limited value.
   *
   * @example
   * @code
   * VenkatakrishnanLimiter<Real> venkatakrishnan;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real max_value = 1.0;
   * Real min_value = 0.0;
   * FaceInfo fi;
   * bool fi_elem_is_upwind = true;
   * Real result = venkatakrishnan.limit(0.0, 0.0, &grad_upwind, nullptr, dCD, max_value, min_value,
   * &fi, fi_elem_is_upwind);
   * @endcode
   */
  T limit(const T & phi_upwind,
          const T & /* phi_downwind */,
          const VectorValue<T> * grad_phi_upwind,
          const VectorValue<T> * /* grad_phi_downwind */,
          const RealVectorValue & /* dCD */,
          const T & max_value,
          const T & min_value,
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

  /**
   * @brief Indicates whether the Venkatakrishnan limiter is constant.
   *
   * This method always returns `false` as the Venkatakrishnan limiter is not a constant limiter.
   *
   * @return `false` indicating the Venkatakrishnan limiter is not constant.
   */
  bool constant() const override final { return false; }

  /**
   * @brief Returns the interpolation method used by the Venkatakrishnan limiter.
   *
   * This method returns `InterpMethod::SOU`, indicating the interpolation method used.
   *
   * @return The interpolation method `InterpMethod::SOU`.
   */
  InterpMethod interpMethod() const override final { return InterpMethod::SOU; }

  /**
   * @brief Default constructor for the Venkatakrishnan limiter.
   */
  VenkatakrishnanLimiter() = default;
};
}
}
