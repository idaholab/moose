//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  return rot2_to_z.transpose() * rot1_to_z;
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
}
