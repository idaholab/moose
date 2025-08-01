//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "MooseTypes.h"
/**
 * Utility functions to return rotations matrics
 */
namespace RotationMatrix
{
/// provides a rotation matrix that will rotate the vector vec to the z axis (the "2" direction)
template <bool is_ad = false>
GenericRealTensorValue<is_ad>
rotVecToZ(GenericRealVectorValue<is_ad> vec)
// provides a rotation matrix that will rotate the vector vec to the z axis (the "2" direction)
{
  // ensure that vec is normalised
  vec /= std::sqrt(vec * vec);

  // construct v0 and v1 to be orthonormal to vec
  // and form a RH basis, that is, so v1 x vec = v0

  // Use Gram-Schmidt method to find v1.
  GenericRealVectorValue<is_ad> v1;
  // Need a prototype for v1 first, and this is done by looking at the smallest component of vec
  GenericRealVectorValue<is_ad> w(std::abs(vec(0)), std::abs(vec(1)), std::abs(vec(2)));
  if ((w(2) >= w(1) && w(1) >= w(0)) || (w(1) >= w(2) && w(2) >= w(0)))
    // vec(0) is the smallest component
    v1(0) = 1;
  else if ((w(2) >= w(0) && w(0) >= w(1)) || (w(0) >= w(2) && w(2) >= w(1)))
    // vec(1) is the smallest component
    v1(1) = 1;
  else
    // vec(2) is the smallest component
    v1(2) = 1;
  // now Gram-Schmidt
  v1 -= (v1 * vec) * vec;
  v1 /= std::sqrt(v1 * v1);

  // now use v0 = v1 x vec
  GenericRealVectorValue<is_ad> v0;
  v0(0) = v1(1) * vec(2) - v1(2) * vec(1);
  v0(1) = v1(2) * vec(0) - v1(0) * vec(2);
  v0(2) = v1(0) * vec(1) - v1(1) * vec(0);

  // the desired rotation matrix is just
  GenericRealTensorValue<is_ad> rot(
      v0(0), v0(1), v0(2), v1(0), v1(1), v1(2), vec(0), vec(1), vec(2));
  return rot;
}

/// provides a rotation matrix that will rotate the vector vec1 to vec2
template <bool is_ad = false>
GenericRealTensorValue<is_ad>
rotVec1ToVec2(GenericRealVectorValue<is_ad> vec1, GenericRealVectorValue<is_ad> vec2)
// provides a rotation matrix that will rotate the vector vec1 to the vector vec2
{
  GenericRealTensorValue<is_ad> rot1_to_z = rotVecToZ<is_ad>(vec1);
  GenericRealTensorValue<is_ad> rot2_to_z = rotVecToZ<is_ad>(vec2);
  GenericRealTensorValue<is_ad> result = rot2_to_z.transpose() * rot1_to_z;
  return result;
}

/// provides a rotation matrix that will rotate the vector vec1 to the [1,0,0], assuming vec1[2]==0
template <bool is_ad = false>
GenericRealTensorValue<is_ad>
rotVec2DToX(const GenericRealVectorValue<is_ad> & vec)
// provides a rotation matrix that will rotate the vector `vec` to the [1,0,0], assuming vec[2]==0
{
  const GenericReal<is_ad> theta = std::atan2(vec(1), vec(0));
  const GenericReal<is_ad> st = std::sin(theta);
  const GenericReal<is_ad> ct = std::cos(theta);
  return GenericRealTensorValue<is_ad>(ct, st, 0., -st, ct, 0., 0., 0., 1.);
}

/// @brief Provides rotatiom matrix for rotating vec1 to vec2 using Rodrigues' rotation forumula. See https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula#Matrix_notation
/// @param vec1 starting vector -- must have 3 components!
/// @param vec2 ending vector -- must have 3 components!
/// @return 3x3 rotation tensor (matrix)
template <bool is_ad = false>
GenericRealTensorValue<is_ad>
rodriguesRotationMatrix(GenericRealVectorValue<is_ad> vec1, GenericRealVectorValue<is_ad> vec2)
{
  // normalize input vectors
  GenericRealVectorValue<is_ad> u = vec1 / vec1.norm();
  GenericRealVectorValue<is_ad> v = vec2 / vec2.norm();

  if ((u - v).norm() < libMesh::TOLERANCE)
    return GenericRealTensorValue<is_ad>(1, 0, 0, 0, 1, 0, 0, 0, 1); // identity matrix

  GenericRealVectorValue<is_ad> k_vec = u.cross(v); // calculate rotation axis
  k_vec /= k_vec.norm();                            // normalize
  Real cos_theta = u * v;
  Real theta = std::acos(cos_theta);
  Real sin_theta = std::sin(theta);

  GenericRealTensorValue<is_ad> K_matrix(
      0, -k_vec(2), k_vec(1), k_vec(2), 0, -k_vec(0), -k_vec(1), k_vec(0), 0);
  GenericRealTensorValue<is_ad> I(1, 0, 0, 0, 1, 0, 0, 0, 1); // identity matrix

  GenericRealTensorValue<is_ad> rot_matrix;
  rot_matrix =
      I + sin_theta * K_matrix + (1 - cos_theta) * K_matrix * K_matrix; // construct rotation matrix
  return rot_matrix;
}
}
