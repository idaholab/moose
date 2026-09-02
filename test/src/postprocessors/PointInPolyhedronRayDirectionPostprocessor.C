//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronRayDirectionPostprocessor.h"
#include "PointInPolyhedronCheckUO.h"

registerMooseObject("MooseTestApp", PointInPolyhedronRayDirectionPostprocessor);

InputParameters
PointInPolyhedronRayDirectionPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports one component of the resolved ray direction of a "
                             "PointInPolyhedronCheckUO (test-only).");
  params.addRequiredParam<UserObjectName>(
      "user_object", "The PointInPolyhedronCheckUO whose resolved ray direction is reported.");
  params.addParam<MooseEnum>(
      "component", MooseEnum("x y z", "x"), "Component of the ray direction to report.");
  return params;
}

PointInPolyhedronRayDirectionPostprocessor::PointInPolyhedronRayDirectionPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _uo(getUserObject<PointInPolyhedronCheckUO>("user_object")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
PointInPolyhedronRayDirectionPostprocessor::getValue() const
{
  return _uo.rayDirection()(_component);
}
