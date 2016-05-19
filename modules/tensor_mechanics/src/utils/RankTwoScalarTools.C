/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MooseTypes.h"
#include "RankTwoTensor.h"
#include "RankTwoScalarTools.h"

namespace RankTwoScalarTools
{

Real
component(const RankTwoTensor & r2tensor, unsigned int i, unsigned int j)
{
  return r2tensor(i,j);
}

Real
component(const RankTwoTensor & r2tensor, unsigned int i, unsigned int j, Point & direction)
{
  direction.zero();
  if (i == j)
    direction(i) = 1.0;

  else
  {
    direction(i) = std::sqrt(0.5);
    direction(j) = std::sqrt(0.5);
  }
  return r2tensor(i,j);
}

Real
vonMisesStress(const RankTwoTensor & stress)
{
  RankTwoTensor dev_stress = stress.deviatoric();
  return std::pow(3.0 / 2.0 * dev_stress.doubleContraction(dev_stress), 0.5);
}

Real
equivalentPlasticStrain(const RankTwoTensor & strain)
{
  return std::pow(2.0 / 3.0 * strain.doubleContraction(strain), 0.5);
}

Real
hydrostatic(const RankTwoTensor & r2tensor)
{
  return r2tensor.trace() / 3.0;
}

Real
L2norm(const RankTwoTensor & r2tensor)
{
  return r2tensor.L2norm();
}

Real
volumetricStrain(const RankTwoTensor & strain)
{
  Real val = strain.trace();
  for (unsigned int i = 0; i < 2; ++i)
  {
    for (unsigned int j = i + 1; j < 3; ++j)
      val += strain(i,i) * strain(j,j);
  }
  val += strain(0,0) * strain(1,1) * strain(2,2);
  return val;
}

Real
firstInvariant(const RankTwoTensor & r2tensor)
{
  return r2tensor.trace();
}

Real
secondInvariant(const RankTwoTensor & r2tensor)
{
  Real val = 0.0;
  for (unsigned int i = 0; i < 2; ++i)
  {
    for (unsigned int j = i + 1; j < 3; ++j)
    {
      val += r2tensor(i,i) * r2tensor(j,j);
      val -= (r2tensor(i,j) * r2tensor(i,j) + r2tensor(j,i) * r2tensor(j,i)) * 0.5;
    }
  }
  return val;
}

Real
thirdInvariant(const RankTwoTensor & r2tensor)
{
  Real val = 0.0;
  val = r2tensor(0,0) * r2tensor(1,1) * r2tensor(2,2) -
        r2tensor(0,0) * r2tensor(1,2) * r2tensor(2,1) +
        r2tensor(0,1) * r2tensor(1,2) * r2tensor(2,0) -
        r2tensor(0,1) * r2tensor(1,0) * r2tensor(2,2) +
        r2tensor(0,2) * r2tensor(1,0) * r2tensor(2,1) -
        r2tensor(0,2) * r2tensor(1,1) * r2tensor(2,0);

  return val;
}

Real
maxPrinciple(const RankTwoTensor & r2tensor)
{
  Real val = calcEigenValues(r2tensor, (LIBMESH_DIM - 1));
  return val;
}

Real
midPrinciple(const RankTwoTensor & r2tensor)
{
  Real val = calcEigenValues(r2tensor, 1);
  return val;
}

Real
minPrinciple(const RankTwoTensor & r2tensor)
{
  Real val = calcEigenValues(r2tensor, 0);
  return val;
}

Real
calcEigenValues(const RankTwoTensor & r2tensor, unsigned int index)
{
  std::vector<Real> eigenval(LIBMESH_DIM);
  r2tensor.symmetricEigenvalues(eigenval);

  Real val = eigenval[index];

  return val;
}

Real
axialStress(const RankTwoTensor & stress, const Point & point1, const Point & point2, RealVectorValue & direction)
{
  Point axis = point2 - point1;
  axis /= axis.norm();

  // Calculate the stress in the direction of the axis specifed by the user
  Real axial_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      axial_stress += axis(j) * stress(j, i) * axis(i);
  }
  direction = axis;
  return axial_stress;
}

Real
hoopStress(const RankTwoTensor & stress, const Point & point1, const Point & point2, const Point * curr_point, RealVectorValue & direction)
{
  // Calculate the cross of the normal to the axis of rotation from the current
  // location and the axis of rotation
  Point xp;
  normalPositionVector(point1, point2, curr_point, xp);
  Point axis_rotation = point2 - point1;
  Point yp = axis_rotation / axis_rotation.norm();
  Point zp = xp.cross(yp);

  // Calculate the scalar value of the hoop stress
  Real hoop_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      hoop_stress += zp(j) * stress(j, i) * zp(i);
  }
  direction = zp;
  return hoop_stress;
}

Real
radialStress(const RankTwoTensor & stress, const Point & point1, const Point & point2, const Point * curr_point, RealVectorValue & direction)
{
  Point radial_norm;
  normalPositionVector(point1, point2, curr_point, radial_norm);

  // Compute the scalar stress component in the direciton of the normal vector from the
  // user-defined axis of rotation.
  Real radial_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      radial_stress += radial_norm(j) * stress(j, i) * radial_norm(i);
  }
  direction = radial_norm;
  return radial_stress;
}

void
normalPositionVector(const Point & point1, const Point & point2, const Point * curr_point, Point & normalPosition)
{
  // Find the current location
  Point position = (* curr_point);

  // Find the nearest point on the axis of rotation (defined by point2 - point1)
  // to the current position, e.g. the normal to the axis of rotation at the
  // current position
  Point axis_rotation = point2 - point1;
  Point positionWRTpoint1 = point1 - position;
  Real projection =  (axis_rotation * positionWRTpoint1) / axis_rotation.norm_sq();
  Point normal = point1 - projection * axis_rotation;

  // Calculate the direction normal to the plane formed by the axis of rotation
  // and the normal to the axis of rotation from the current position.
  normalPosition = position - normal;
  normalPosition /= normalPosition.norm();
}

Real
directionValueTensor(const RankTwoTensor & r2tensor, Point & input_direction)
{
  Real tensor_value_in_direction = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      tensor_value_in_direction += input_direction(j) * r2tensor(j, i) * input_direction(i);
  }
  return tensor_value_in_direction;
}

Real
triaxialityStress(const RankTwoTensor & stress)
{
  Real val = hydrostatic(stress) / vonMisesStress(stress);
  return val;
}

}
