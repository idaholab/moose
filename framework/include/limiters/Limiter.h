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
   * This method computes the flux limiting ratio based on the provided scalar values,
   * gradient vectors, direction vector, maximum and minimum allowable values, and
   * geometric information from the face and cell centroids. It must be overridden
   * by any derived class implementing a specific limiting strategy.
   *
   * @tparam T The data type of the field values and the return type.
   * @param phi_upwind The field value at the upwind location.
   * @param phi_downwind The field value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient of the field at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient of the field at the downwind location.
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
   */
  virtual T limit(const T & phi_upwind,
                  const T & phi_downwind,
                  const libMesh::VectorValue<T> * grad_phi_upwind,
                  const libMesh::VectorValue<T> * grad_phi_downwind,
                  const RealVectorValue & dCD,
                  const Real & max_value,
                  const Real & min_value,
                  const FaceInfo * fi,
                  const bool & fi_elem_is_upwind) const = 0;
  virtual bool constant() const = 0;
  virtual InterpMethod interpMethod() const = 0;

  /**
   * @tparam T The data type of the field values and the return type.
   * @param phi_upwind The field value at the upwind location.
   * @param phi_downwind The field value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient of the field value at the upwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed limited value, ensuring it is within the range [0, 2].
   */
  T operator()(const T & phi_upwind,
               const T & phi_downwind,
               const libMesh::VectorValue<T> * grad_phi_upwind,
               const RealVectorValue & dCD) const
  {
    return std::max(T(0),
                    std::min(T(2),
                             limit(phi_upwind,
                                   phi_downwind,
                                   grad_phi_upwind,
                                   nullptr,
                                   dCD,
                                   T(0),
                                   T(0),
                                   nullptr,
                                   false)));
  }

  /**
   * @tparam T The data type of the field values and the return type.
   * @param phi_upwind The field value at the upwind location.
   * @param phi_downwind The field value at the downwind location.
   * @param grad_phi_upwind Pointer to the gradient at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @param max_value The maximum allowable value.
   * @param min_value The minimum allowable value.
   * @param fi FaceInfo object containing geometric details such as face centroid and cell
   * centroids.
   * @param fi_elem_is_upwind Boolean indicating if the face info element is upwind.
   * @return The result of the `limit` function applied to the provided parameters.
   */
  T operator()(const T & phi_upwind,
               const T & phi_downwind,
               const VectorValue<T> * grad_phi_upwind,
               const VectorValue<T> * grad_phi_downwind,
               const RealVectorValue & dCD,
               const Real & max_value,
               const Real & min_value,
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
   * @tparam T The data type of the gradient values and the return type.
   * @param grad_phi_upwind Pointer to the gradient at the upwind location.
   * @param grad_phi_downwind Pointer to the gradient at the downwind location.
   * @param dCD A constant direction vector representing the direction of the cell face.
   * @return The computed flux limiting ratio.
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
   * @tparam T The data type of the field values and the return type.
   * @param phi_upwind The field value at the upwind location.
   * @param grad_phi_upwind Pointer to the gradient at the upwind location.
   * @param max_value The maximum allowable value.
   * @param min_value The minimum allowable value.
   * @param fi FaceInfo object containing geometric details such as face centroid and cell
   * centroids.
   * @param fi_elem_is_upwind Boolean indicating if the face info element is upwind.
   * @return The computed flux limiting ratio.
   */
  T rf_minmax(const T & phi_upwind,
              const VectorValue<T> * grad_phi_upwind,
              const Real & max_value,
              const Real & min_value,
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
