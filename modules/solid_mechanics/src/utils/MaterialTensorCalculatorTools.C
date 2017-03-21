/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaterialTensorCalculatorTools.h"

namespace MaterialTensorCalculatorTools
{

Real
component(const SymmTensor & symm_tensor, unsigned int index)
{
  return symm_tensor.component(index);
}

Real
component(const SymmTensor & symm_tensor, unsigned int index, RealVectorValue & direction)
{
  direction.zero();
  if (index < 3)
    direction(index) = 1.0;

  else if (index == 3) // xy
  {
    direction(0) = std::sqrt(0.5);
    direction(1) = direction(0);
  }
  else if (index == 4) // yz
  {
    direction(1) = std::sqrt(0.5);
    direction(2) = direction(1);
  }
  else if (index == 5) // zx
  {
    direction(0) = std::sqrt(0.5);
    direction(2) = direction(0);
  }
  return symm_tensor.component(index);
}

Real
vonMisesStress(const SymmTensor & symm_stress)
{
  Real value = std::pow(symm_stress.xx() - symm_stress.yy(), 2) +
               std::pow(symm_stress.yy() - symm_stress.zz(), 2) +
               std::pow(symm_stress.zz() - symm_stress.xx(), 2) +
               6 * (std::pow(symm_stress.xy(), 2) + std::pow(symm_stress.yz(), 2) +
                    std::pow(symm_stress.zx(), 2));
  return std::sqrt(value / 2.0);
}

Real
equivalentPlasticStrain(const SymmTensor & symm_strain)
{
  return std::sqrt(2.0 / 3.0 * symm_strain.doubleContraction(symm_strain));
}

Real
hydrostatic(const SymmTensor & symm_tensor)
{
  return symm_tensor.trace() / 3.0;
}

Real
volumetricStrain(const SymmTensor & symm_strain)
{
  Real value = symm_strain.trace();
  value += symm_strain.xx() * symm_strain.yy() + symm_strain.yy() * symm_strain.zz() +
           symm_strain.zz() * symm_strain.xx() +
           symm_strain.xx() * symm_strain.yy() * symm_strain.zz();
  return value;
}

Real
firstInvariant(const SymmTensor & symm_tensor)
{
  return symm_tensor.trace();
}

Real
secondInvariant(const SymmTensor & symm_tensor)
{
  Real value = symm_tensor.xx() * symm_tensor.yy() + symm_tensor.yy() * symm_tensor.zz() +
               symm_tensor.zz() * symm_tensor.xx() - symm_tensor.xy() * symm_tensor.xy() -
               symm_tensor.yz() * symm_tensor.yz() - symm_tensor.zx() * symm_tensor.zx();

  return value;
}

Real
thirdInvariant(const SymmTensor & symm_tensor)
{
  Real val = 0.0;
  val = symm_tensor.xx() * symm_tensor.yy() * symm_tensor.zz() -
        symm_tensor.xx() * symm_tensor.yz() * symm_tensor.yz() +
        symm_tensor.xy() * symm_tensor.yz() * symm_tensor.zx() -
        symm_tensor.xy() * symm_tensor.xy() * symm_tensor.zz() +
        symm_tensor.zx() * symm_tensor.xy() * symm_tensor.yz() -
        symm_tensor.zx() * symm_tensor.yy() * symm_tensor.zx();

  return val;
}

Real
maxPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction)
{
  Real val = calcPrincipleValues(symm_tensor, (LIBMESH_DIM - 1), direction);
  return val;
}

Real
midPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction)
{
  Real val = calcPrincipleValues(symm_tensor, 1, direction);
  return val;
}

Real
minPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction)
{
  Real val = calcPrincipleValues(symm_tensor, 0, direction);
  return val;
}

Real
calcPrincipleValues(const SymmTensor & symm_tensor, unsigned int index, RealVectorValue & direction)
{
  ColumnMajorMatrix eval(3, 1);
  ColumnMajorMatrix evec(3, 3);
  symm_tensor.columnMajorMatrix().eigen(eval, evec);
  // Eigen computes low to high.  We want high first.
  direction(0) = evec(0, index);
  direction(1) = evec(1, index);
  direction(2) = evec(2, index);
  return eval(index);
}

Real
axialStress(const SymmTensor & symm_stress,
            const Point & point1,
            const Point & point2,
            RealVectorValue & direction)
{
  Point axis = point2 - point1;
  axis /= axis.norm();

  // Calculate the stress in the direction of the axis specifed by the user
  Real axial_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      axial_stress += axis(j) * symm_stress(j, i) * axis(i);
  }
  direction = axis;
  return axial_stress;
}

Real
hoopStress(const SymmTensor & symm_stress,
           const Point & point1,
           const Point & point2,
           const Point & curr_point,
           RealVectorValue & direction)
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
      hoop_stress += zp(j) * symm_stress(j, i) * zp(i);
  }
  direction = zp;
  return hoop_stress;
}

Real
radialStress(const SymmTensor & symm_stress,
             const Point & point1,
             const Point & point2,
             const Point & curr_point,
             RealVectorValue & direction)
{
  Point radial_norm;
  normalPositionVector(point1, point2, curr_point, radial_norm);

  // Compute the scalar stress component in the direciton of the normal vector from the
  // user-defined axis of rotation.
  Real radial_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      radial_stress += radial_norm(j) * symm_stress(j, i) * radial_norm(i);
  }
  direction = radial_norm;
  return radial_stress;
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

Real
directionValueTensor(const SymmTensor & symm_tensor, const RealVectorValue & input_direction)
{
  Real tensor_value_in_direction = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      tensor_value_in_direction += input_direction(j) * symm_tensor(j, i) * input_direction(i);
  }
  return tensor_value_in_direction;
}

Real
triaxialityStress(const SymmTensor & symm_stress)
{
  Real val = hydrostatic(symm_stress) / vonMisesStress(symm_stress);
  return val;
}
}
