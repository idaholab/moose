//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RotationMatrix.h"

RealTensorValue
RotationMatrix::rotVecToZ(RealVectorValue vec)
// provides a rotation matrix that will rotate the vector vec to the z axis (the "2" direction)
{
  // ensure that vec is normalised
  vec /= std::sqrt(vec * vec);

  // construct v0 and v1 to be orthonormal to vec
  // and form a RH basis, that is, so v1 x vec = v0

  // Use Gram-Schmidt method to find v1.
  RealVectorValue v1;
  // Need a prototype for v1 first, and this is done by looking at the smallest component of vec
  RealVectorValue w(std::abs(vec(0)), std::abs(vec(1)), std::abs(vec(2)));
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
  RealVectorValue v0;
  v0(0) = v1(1) * vec(2) - v1(2) * vec(1);
  v0(1) = v1(2) * vec(0) - v1(0) * vec(2);
  v0(2) = v1(0) * vec(1) - v1(1) * vec(0);

  // the desired rotation matrix is just
  RealTensorValue rot(v0(0), v0(1), v0(2), v1(0), v1(1), v1(2), vec(0), vec(1), vec(2));
  return rot;
}

RealTensorValue
RotationMatrix::rotVec1ToVec2(RealVectorValue vec1, RealVectorValue vec2)
// provides a rotation matrix that will rotate the vector vec1 to the vector vec2
{
  RealTensorValue rot1_to_z = rotVecToZ(vec1);
  RealTensorValue rot2_to_z = rotVecToZ(vec2);
  return rot2_to_z.transpose() * rot1_to_z;
}

RealTensorValue
RotationMatrix::rotVec2DToX(const RealVectorValue & vec)
// provides a rotation matrix that will rotate the vector `vec` to the [1,0,0], assuming vec[2]==0
{
  const Real theta = std::atan2(vec(1), vec(0));
  const Real st = std::sin(theta);
  const Real ct = std::cos(theta);
  return RealTensorValue(ct, st, 0., -st, ct, 0., 0., 0., 1.);
  ;
}
