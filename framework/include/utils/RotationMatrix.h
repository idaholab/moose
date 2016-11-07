/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ROTATIONMATRIX_H
#define ROTATIONMATRIX_H

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
}

#endif // ROTATIONMATRIX_H
