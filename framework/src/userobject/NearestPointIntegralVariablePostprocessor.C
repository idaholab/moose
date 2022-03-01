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

InputParameters
NearestPointIntegralVariablePostprocessor::validParams()
{
  InputParameters params = NearestPointBase<ElementIntegralVariablePostprocessor,
                                            ElementVariableVectorPostprocessor>::validParams();

  params.addClassDescription(
      "Compute element variable integrals for nearest-point based subdomains");

  params.registerBase("VectorPostprocessor");

  return params;
}

NearestPointIntegralVariablePostprocessor::NearestPointIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : NearestPointBase<ElementIntegralVariablePostprocessor, ElementVariableVectorPostprocessor>(
        parameters),
    _np_post_processor_values(declareVector("np_post_processor_values"))
{
  _np_post_processor_values.resize(_user_objects.size());
}

Real
NearestPointIntegralVariablePostprocessor::spatialValue(const Point & point) const
{
  unsigned int i = nearestPointIndex(point);

  if (i >= _np_post_processor_values.size())
    mooseError("The vector length of vector post processor is ",
               _np_post_processor_values.size(),
               " but nearestPointIndex() return ",
               i);

  return _np_post_processor_values[i];
}

Real
NearestPointIntegralVariablePostprocessor::userObjectValue(unsigned int i) const
{
  if (i >= _np_post_processor_values.size())
    mooseError("The vector length of vector post processor is ",
               _np_post_processor_values.size(),
               " but you pass in ",
               i);

  return _np_post_processor_values[i];
}

void
NearestPointIntegralVariablePostprocessor::finalize()
{
  if (_user_objects.size() != _np_post_processor_values.size())
    mooseError("The vector length of the vector postprocessor ",
               _np_post_processor_values.size(),
               " is different from the number of user objects ",
               _user_objects.size());

  unsigned int i = 0;
  for (auto & user_object : _user_objects)
  {
    user_object->finalize();
    _np_post_processor_values[i++] = user_object->getValue();
  }
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
