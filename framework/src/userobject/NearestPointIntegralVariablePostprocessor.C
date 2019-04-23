//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointIntegralVariablePostprocessor.h"

registerMooseObject("MooseApp", NearestPointIntegralVariablePostprocessor);

template <>
InputParameters
validParams<NearestPointIntegralVariablePostprocessor>()
{
  InputParameters params = nearestPointBaseValidParams<ElementIntegralVariablePostprocessor>();

  return params;
}

NearestPointIntegralVariablePostprocessor::NearestPointIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : NearestPointBase<ElementIntegralVariablePostprocessor>(parameters)
{
}

Real
NearestPointIntegralVariablePostprocessor::spatialValue(const Point & point) const
{
  return nearestUserObject(point)->getValue();
}

Real
NearestPointIntegralVariablePostprocessor::userObjectValue(unsigned int i) const
{
  return userObject(i)->getValue();
}

unsigned int
NearestPointIntegralVariablePostprocessor::nearestPointIndex(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for (auto it : Moose::enumerate(_points))
  {
    const auto & current_point = it.value();

    Real current_distance = (p - current_point).norm();

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = it.index();
    }
  }

  return closest;
}

std::shared_ptr<ElementIntegralVariablePostprocessor>
NearestPointIntegralVariablePostprocessor::userObject(unsigned int i) const
{
  return _user_objects[i];
}
