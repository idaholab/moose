//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoScalarTools.h"

namespace RankTwoScalarTools
{

/// This enum is left for legacy calls
MooseEnum
scalarOptions()
{
  return MooseEnum("VonMisesStress EffectiveStrain Hydrostatic L2norm MaxPrincipal "
                   "MidPrincipal MinPrincipal VolumetricStrain FirstInvariant SecondInvariant "
                   "ThirdInvariant AxialStress HoopStress RadialStress TriaxialityStress "
                   "Direction MaxShear StressIntensity");
}

MooseEnum
invariantOptions()
{
  return MooseEnum("VonMisesStress EffectiveStrain Hydrostatic L2norm VolumetricStrain "
                   "FirstInvariant SecondInvariant "
                   "ThirdInvariant TriaxialityStress MaxShear StressIntensity EffectiveStrain");
}

MooseEnum
cylindricalOptions()
{
  return MooseEnum("AxialStress HoopStress RadialStress");
}

MooseEnum
sphericalOptions()
{
  return MooseEnum("HoopStress RadialStress");
}

void
normalPositionVector(const Point & point1,
                     const Point & point2,
                     const Point & curr_point,
                     Point & normalPosition)
{
  // Find the nearest point on the axis of rotation (defined by point2 - point1)
  // to the current position, e.g. the normal to the axis of rotation at the
  // current position
  Point axis_rotation = point2 - point1;
  Point positionWRTpoint1 = point1 - curr_point;
  Real projection = (axis_rotation * positionWRTpoint1) / axis_rotation.norm_sq();
  Point normal = point1 - projection * axis_rotation;

  // Calculate the direction normal to the plane formed by the axis of rotation
  // and the normal to the axis of rotation from the current position.
  normalPosition = curr_point - normal;
  normalPosition /= normalPosition.norm();
}

void
setRotationMatrix(const RealVectorValue & outwardnormal,
                  const RealVectorValue & axialVector,
                  RankTwoTensor & rotationMatrix,
                  const bool transpose)
{
  RealVectorValue radial_vector(outwardnormal);
  RealVectorValue azimuthal_vector(0, 0, 0);
  RealVectorValue polar_vector(axialVector);

  Real rv_norm = radial_vector.norm();
  if (rv_norm)
    radial_vector /= rv_norm;
  else
    mooseError("The outward normal vector cannot be a zero vector");

  azimuthal_vector = polar_vector.cross(radial_vector);
  azimuthal_vector /= azimuthal_vector.norm();

  if (!transpose)
  {
    // Considering that Moose uses convention [T'] = [Q][T][Q_transpose], the
    // basis vectors of old coordinate system should be arranged column wise
    // to construct the rotation matrix Q.

    rotationMatrix.fillColumn(0, radial_vector);
    rotationMatrix.fillColumn(1, azimuthal_vector);
    rotationMatrix.fillColumn(2, polar_vector);
  }
  else
  {
    rotationMatrix.fillRow(0, radial_vector);
    rotationMatrix.fillRow(1, azimuthal_vector);
    rotationMatrix.fillRow(2, polar_vector);
  }
}
}
