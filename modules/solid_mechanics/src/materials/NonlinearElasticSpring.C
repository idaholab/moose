//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearElasticSpring.h"

#include "Function.h"

registerMooseObject("SolidMechanicsApp", NonlinearElasticSpring);

InputParameters
NonlinearElasticSpring::validParams()
{
  InputParameters params = SpringLawBase::validParams();
  params.addClassDescription("Nonlinear elastic spring law: the force along each local axis is "
                             "a function of the deformation along that axis, the same curve on "
                             "loading and unloading.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "backbone_functions",
      "Three functions, one per local axis, giving the force (dimension force, or force per "
      "area when tributary_area is given; moment in the rotational mode) as a function of the "
      "deformation along that axis (dimension length, or radian); the deformation is the x "
      "coordinate of the function, so a PiecewiseLinear function needs axis = x and a "
      "ParsedFunction uses x. "
      "A tension-only spring has a zero branch for negative x, a compression-only spring for "
      "positive x, and a gap a zero branch between the two deformations that close it; an "
      "axis with no stiffness needs some other object holding its degree of freedom.");
  return params;
}

NonlinearElasticSpring::NonlinearElasticSpring(const InputParameters & parameters)
  : SpringLawBase(parameters)
{
  const auto & names = getParam<std::vector<FunctionName>>("backbone_functions");
  if (names.size() != 3)
    paramError("backbone_functions", "Three functions are needed, one per local axis.");
  for (const auto & name : names)
    _backbones.push_back(&getFunctionByName(name));
}

void
NonlinearElasticSpring::computeLaw(const RealVectorValue & deformation,
                                   RealVectorValue & force,
                                   RankTwoTensor & stiffness)
{
  stiffness.zero();
  for (unsigned int axis = 0; axis < 3; ++axis)
  {
    const Point along_axis(deformation(axis), 0.0, 0.0);
    force(axis) = _backbones[axis]->value(_t, along_axis);
    stiffness(axis, axis) = _backbones[axis]->gradient(_t, along_axis)(0);
  }
}
