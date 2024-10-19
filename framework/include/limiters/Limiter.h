//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADReal.h"
#include "MooseTypes.h"
#include "HasMembers.h"
#include "FaceInfo.h"
#include <memory>

class MooseEnum;

namespace Moose
{
namespace FV
{
enum class InterpMethod;

enum class LimiterType : int
{
  VanLeer = 0,
  Upwind,
  CentralDifference,
  MinMod,
  SOU,
  QUICK,
  Venkatakrishnan
};
extern const MooseEnum moose_limiter_type;

template <typename T, typename Enable = void>
struct LimiterValueType;

template <>
struct LimiterValueType<Real>
{
  typedef Real value_type;
};
template <>
struct LimiterValueType<ADReal>
{
  typedef ADReal value_type;
};
template <typename T>
struct LimiterValueType<T, typename std::enable_if<HasMemberType_value_type<T>::value>::type>
{
  typedef typename T::value_type value_type;
};

/**
 * Base class for defining slope limiters for finite volume or potentially reconstructed
 * Discontinuous-Galerkin applications
 */
template <typename T>
class Limiter
{
public:
  /**
   * @brief Pure virtual method for computing the flux limiting ratio.
   *
   * This method computes the flux limiting ratio based on the provided scalar values,
   * gradient vectors, direction vector, maximum and minimum allowable values, and
   * geometric information from the face and cell centroids. It must be overridden
   * by any derived class implementing a specific limiting strategy.
   *
   * @tparam T The data type of the scalar values and the return type.
   * @param phi_upwind The scalar value at the upwind location.
   * @param phi_downwind The scalar value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @param max_value The maximum allowable value.
   * @param min_value The minimum allowable value.
   * @param fi FaceInfo object containing geometric details such as face centroid and cell
   * centroids.
   * @param fi_elem_is_upwind Boolean indicating if the face info element is upwind.
   * @return The computed flux limiting ratio.
   *
   * This pure virtual function is intended to be defined in derived classes, which will implement
   * the specific limiting algorithm. Derived classes will provide the actual logic for computing
   * the flux limiting ratio, ensuring that the result adheres to the constraints and properties
   * required by the specific limiting method.
   *
   * Here is an example of how a derived class might implement this method:
   *
   * @example
   * @code
   * class MyLimiter : public Limiter<Real>
   * {
   * public:
   *     Real limit(const Real & phi_upwind,
   *                const Real & phi_downwind,
   *                const VectorValue<Real> * grad_phi_upwind,
   *                const VectorValue<Real> * grad_phi_downwind,
   *                const RealVectorValue & dCD,
   *                const Real & max_value,
   *                const Real & min_value,
   *                const FaceInfo * fi,
   *                const bool & fi_elem_is_upwind) const override
   *     {
   *         // Implementation of the specific limiting algorithm.
   *         Real ratio = ... // Compute the ratio.
   *         return ratio; // Return the computed ratio.
   *     }
   * };
   * @endcode
   */
  virtual T limit(const T & phi_upwind,
                  const T & phi_downwind,
                  const VectorValue<T> * grad_phi_upwind,
                  const VectorValue<T> * grad_phi_downwind,
                  const RealVectorValue & dCD,
                  const T & max_value,
                  const T & min_value,
                  const FaceInfo * fi,
                  const bool & fi_elem_is_upwind) const = 0;
  virtual bool constant() const = 0;
  virtual InterpMethod interpMethod() const = 0;

  /**
   * @brief Functor for applying tsimplified slope limiting.
   *
   * This function applies the limiter by invoking the `limit` method with the provided parameters.
   * It acts as a functor, enabling objects of the `VanLeerLimiter` class to be used as if they were
   * functions.
   *
   * @tparam T The data type of the scalar values and the return type.
   * @param phi_upwind The scalar value at the upwind location.
   * @param phi_downwind The scalar value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed limited value, ensuring it is within the range [0, 2].
   *
   * This method performs the following steps:
   * 1. Calls the `limit` method with the provided scalar values, the upwind gradient vector, and
   * the direction vector.
   * 2. Ensures the result is within the range [0, 2] by using `std::max` and `std::min`.
   * 3. Returns the computed limited value.
   *
   * @example
   * @code
   * Limiter<Real> * myLimiter = ... // Assume this is properly initialized
   * Real phi_upwind = 2.0;
   * Real phi_downwind = 1.5;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * FaceInfo * fi = ... // Assume this is properly initialized
   * Real result = (*myLimiter)(phi_upwind, phi_downwind, &grad_upwind, dCD);
   * @endcode
   */
  T operator()(const T & phi_upwind,
               const T & phi_downwind,
               const VectorValue<T> * grad_phi_upwind,
               const RealVectorValue & dCD) const
  {
    return std::max(T(0),
                    std::min(T(2),
                             limit(phi_upwind,
                                   phi_downwind,
                                   grad_phi_upwind,
                                   nullptr,
                                   dCD,
                                   0.0,
                                   0.0,
                                   nullptr,
                                   false)));
  }

  /**
   * @brief Functor for applying general slope limiter.
   *
   * This function applies the slope limiter by invoking the `limit` method with the provided
   * parameters. It acts as a functor, enabling objects of the `Limiter` class to be used as if they
   * were functions.
   *
   * @tparam T The data type of the scalar values and the return type.
   * @param phi_upwind The scalar value at the upwind location.
   * @param phi_downwind The scalar value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @param max_value The maximum allowable value.
   * @param min_value The minimum allowable value.
   * @param fi FaceInfo object containing geometric details such as face centroid and cell
   * centroids.
   * @param fi_elem_is_upwind Boolean indicating if the face info element is upwind.
   * @return The result of the `limit` function applied to the provided parameters.
   *
   * This function performs the following steps:
   * 1. Calls the `limit` method with the provided scalar values, gradient vectors, direction
   * vector, maximum and minimum values, face information, and upwind status.
   * 2. Returns the result of the `limit` method.
   *
   * This functor allows for more intuitive and flexible usage of the `Limiter` class, enabling
   * instances of the class to be called with arguments directly.
   *
   * @example
   * @code
   * Limiter<Real> * myLimiter = ... // Assume this is properly initialized
   * Real phi_upwind = 2.0;
   * Real phi_downwind = 1.5;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * VectorValue<Real> grad_downwind(0.4, 0.5, 0.6);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real max_value = 5.0;
   * Real min_value = 1.0;
   * FaceInfo * fi = ... // Assume this is properly initialized
   * bool is_upwind = true;
   * Real result = (*myLimiter)(phi_upwind, phi_downwind, &grad_upwind, &grad_downwind, dCD,
   * max_value, min_value, fi, is_upwind);
   * @endcode
   */
  T operator()(const T & phi_upwind,
               const T & phi_downwind,
               const VectorValue<T> * grad_phi_upwind,
               const VectorValue<T> * grad_phi_downwind,
               const RealVectorValue & dCD,
               const T & max_value,
               const T & min_value,
               const FaceInfo * fi,
               const bool & fi_elem_is_upwind) const
  {
    return limit(phi_upwind,
                 phi_downwind,
                 grad_phi_upwind,
                 grad_phi_downwind,
                 dCD,
                 max_value,
                 min_value,
                 fi,
                 fi_elem_is_upwind);
  }

  /**
   * @brief Computes the flux limiting ratio using gradients.
   *
   * This function calculates the flux limiting ratio based on the provided gradients
   * at the upwind and downwind locations, along with a direction vector.
   *
   * @tparam T The data type of the gradient values and the return type.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient vector at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed flux limiting ratio.
   *
   * This function performs the following steps:
   * 1. Computes the dot product of the upwind gradient vector with the direction vector `dCD`.
   * 2. Computes the dot product of the downwind gradient vector with the direction vector `dCD`.
   * 3. Calculates the ratio of the upwind gradient dot product to the downwind gradient dot
   * product, adding a small epsilon value to the denominator to prevent division by zero.
   * 4. Applies a flux limiting formula to this ratio to ensure it remains within a physically
   *    reasonable range, specifically ensuring non-negative values.
   *
   * @note The small epsilon value `1e-10` is added to the denominator to avoid division by zero
   *       and numerical instability.
   *
   * @example
   * @code
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * VectorValue<Real> grad_downwind(0.4, 0.5, 0.6);
   * RealVectorValue dCD(1.0, 0.0, 0.0);
   * Real ratio = rf_grad(&grad_upwind, &grad_downwind, dCD);
   * @endcode
   */
  T rf_grad(const VectorValue<T> * grad_phi_upwind,
            const VectorValue<T> * grad_phi_downwind,
            const RealVectorValue & dCD) const
  {
    const auto grad_elem = (*grad_phi_upwind) * dCD;
    const auto grad_face = (*grad_phi_downwind) * dCD;
    const auto grad_ratio = grad_elem / (grad_face + 1e-10);
    return std::max(2.0 * grad_ratio - 1.0, 0.0);
  };

  /**
   * @brief Computes the flux limiting ratio using successive deltas.
   *
   * This function calculates the flux limiting ratio based on the upwind value,
   * its gradient, maximum and minimum allowable values, and geometric information
   * from the face and cell centroids.
   *
   * @tparam T The data type of the scalar values and the return type.
   * @param phi_upwind The scalar value at the upwind location.
   * @param grad_phi_upwind Pointer to the gradient vector at the upwind location.
   * @param max_value The maximum allowable value.
   * @param min_value The minimum allowable value.
   * @param fi FaceInfo object containing geometric details such as face centroid and cell
   * centroids.
   * @param fi_elem_is_upwind Boolean indicating if the face info element is upwind.
   * @return The computed flux limiting ratio.
   *
   * This function performs the following steps:
   * 1. Retrieves the face centroid from the `FaceInfo` object.
   * 2. Determines the cell centroid based on whether the current element is upwind.
   * 3. Computes the delta value at the face by taking the dot product of the upwind gradient
   *    vector with the difference between the face and cell centroids.
   * 4. Computes the delta values for the maximum and minimum allowable values, adding a small
   *    epsilon value to prevent division by zero.
   * 5. Calculates the flux limiting ratio based on whether the delta value at the face is
   *    non-negative or negative, using the appropriate delta (either max or min).
   *
   * @note The small epsilon value `1e-10` is added to the delta max and delta min values to
   *       avoid division by zero and numerical instability.
   *
   * @example
   * @code
   * Real phi_upwind = 2.0;
   * VectorValue<Real> grad_upwind(0.1, 0.2, 0.3);
   * Real max_value = 5.0;
   * Real min_value = 1.0;
   * FaceInfo * fi = ... // Assume this is properly initialized
   * bool is_upwind = true;
   * Real ratio = rf_minmax(phi_upwind, &grad_upwind, max_value, min_value, fi, is_upwind);
   * @endcode
   */
  T rf_minmax(const T & phi_upwind,
              const VectorValue<T> * grad_phi_upwind,
              const T & max_value,
              const T & min_value,
              const FaceInfo * fi,
              const bool & fi_elem_is_upwind) const
  {
    const auto face_centroid = fi->faceCentroid();
    const auto cell_centroid = fi_elem_is_upwind ? fi->elemCentroid() : fi->neighborCentroid();

    const auto delta_face = (*grad_phi_upwind) * (face_centroid - cell_centroid);
    const auto delta_max = max_value - phi_upwind + 1e-10;
    const auto delta_min = min_value - phi_upwind + 1e-10;

    return delta_face >= 0 ? delta_face / delta_max : delta_face / delta_min;
  };

  Limiter() = default;

  virtual ~Limiter() = default;

  static std::unique_ptr<Limiter> build(LimiterType limiter);
};

/**
 * Return the limiter type associated with the supplied interpolation method
 */
LimiterType limiterType(InterpMethod interp_method);
}
}
