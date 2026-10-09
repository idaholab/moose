//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "libmesh/vector_value.h"

#include <utility>

/**
 * Finite rigid body kinematics parameterized by a translation vector t and a rotation vector theta
 * (axis times angle, in radians). The rotation is given by the Rodrigues formula
 *
 *   R(theta) d = d + a(s) theta x d + b(s) theta x (theta x d),  s = theta . theta,
 *   a(s) = sin(sqrt(s)) / sqrt(s),  b(s) = (1 - cos(sqrt(s))) / s,
 *
 * and the displacement of a material point X with respect to the reference point X_ref is
 *
 *   U(X) = t + (R(theta) - I) (X - X_ref).
 *
 * In 2D, pass theta = (0, 0, theta_z) to obtain the exact in-plane rotation. The coefficients are
 * written in terms of s and evaluated by their Taylor series for small s, so that they are smooth
 * (and their AD derivatives finite) at theta = 0.
 */
namespace RigidBodyKinematicsUtils
{
/// Coefficients a(s) and b(s) of the Rodrigues formula
template <typename T>
std::pair<T, T>
rodriguesCoefficients(const T & s)
{
  using std::cos;
  using std::sin;
  using std::sqrt;
  // Below this threshold the closed forms lose accuracy to cancellation, while the truncation
  // error of the series (next term ~ s^5 / 11!) is below machine precision.
  if (s < 1e-2)
    return {1.0 - s / 6.0 + s * s / 120.0 - s * s * s / 5040.0 + s * s * s * s / 362880.0,
            0.5 - s / 24.0 + s * s / 720.0 - s * s * s / 40320.0 + s * s * s * s / 3628800.0};
  const T phi = sqrt(s);
  return {sin(phi) / phi, (1.0 - cos(phi)) / s};
}

/**
 * Displacement U = t + (R(theta) - I) (X - X_ref) of the material point X of a rigid body
 * @param translation translation t of the reference point
 * @param theta rotation vector
 * @param X reference (undisplaced) position of the material point
 * @param X_ref reference point about which the body rotates
 */
template <typename T>
libMesh::VectorValue<T>
rigidDisplacement(const libMesh::VectorValue<T> & translation,
                  const libMesh::VectorValue<T> & theta,
                  const Point & X,
                  const Point & X_ref)
{
  const auto [a, b] = rodriguesCoefficients<T>(theta * theta);
  const libMesh::VectorValue<T> d = X - X_ref;
  const libMesh::VectorValue<T> theta_x_d = theta.cross(d);
  return translation + a * theta_x_d + b * theta.cross(theta_x_d);
}
}
