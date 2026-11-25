//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSpring.h"

registerMooseObject("SolidMechanicsApp", LinearSpring);

InputParameters
LinearSpring::validParams()
{
  InputParameters params = SpringLawBase::validParams();
  params.addClassDescription("Linear spring law with one constant stiffness per local axis.");
  params.addRequiredParam<RealVectorValue>(
      "stiffness",
      "The three stiffnesses along local axes 1, 2 and 3, each zero or positive; a zero leaves "
      "that axis free. Dimension force per length (moment per radian in the rotational mode), "
      "divided by the dimension of tributary_area when an area is given.");
  return params;
}

LinearSpring::LinearSpring(const InputParameters & parameters)
  : SpringLawBase(parameters), _stiffness_per_axis(getParam<RealVectorValue>("stiffness"))
{
  for (unsigned int axis = 0; axis < 3; ++axis)
    if (_stiffness_per_axis(axis) < 0.0)
      paramError("stiffness", "The stiffnesses must be zero or positive.");
}

void
LinearSpring::computeLaw(const RealVectorValue & deformation,
                         RealVectorValue & force,
                         RankTwoTensor & stiffness)
{
  stiffness.zero();
  for (unsigned int axis = 0; axis < 3; ++axis)
  {
    force(axis) = _stiffness_per_axis(axis) * deformation(axis);
    stiffness(axis, axis) = _stiffness_per_axis(axis);
  }
}
