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

/**
 * Utility functions to return rotations matrics
 */
namespace RotationMatrix
{
/// provides a rotation matrix that will rotate the vector vec to the z axis (the "2" direction)
RealTensorValue rotVecToZ(RealVectorValue vec);

/// provides a rotation matrix that will rotate the vector vec1 to vec2
RealTensorValue rotVec1ToVec2(RealVectorValue vec1, RealVectorValue vec2);

/// provides a rotation matrix that will rotate the vector vec1 to the [1,0,0], assuming vec1[2]==0
RealTensorValue rotVec2DToX(const RealVectorValue & vec);
}
