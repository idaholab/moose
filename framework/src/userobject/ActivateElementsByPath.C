//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActivateElementsByPath.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", ActivateElementsByPath);

InputParameters
ActivateElementsByPath::validParams()
{
  InputParameters params = ActivateElementsUserObjectBase::validParams();

  params.addParam<FunctionName>(
      "function_x", "0", "The x component of the heating spot travel path");
  params.addParam<FunctionName>(
      "function_y", "0", "The y component of the heating spot travel path");
  params.addParam<FunctionName>(
      "function_z", "0", "The z component of the heating spot travel path");
  params.addParam<Real>("activate_distance",
                        1e-4,
                        "The maximum distance of the activated element to the point on the path.");
  return params;
}

ActivateElementsByPath::ActivateElementsByPath(const InputParameters & parameters)
  : ActivateElementsUserObjectBase(parameters),
    _function_x(isParamValid("function_x") ? &getFunction("function_x") : nullptr),
    _function_y(isParamValid("function_y") ? &getFunction("function_y") : nullptr),
    _function_z(isParamValid("function_z") ? &getFunction("function_z") : nullptr),
    _activate_distance(
        declareRestartableData<Real>("activate_distance", getParam<Real>("activate_distance")))
{
}

bool
ActivateElementsByPath::isElementActivated()
{
  // activate center (assume position of the activate center is only time dependent)
  const static Point dummy;
  Real x_t = _function_x ? _function_x->value(_t, dummy) : 0.0;
  Real y_t = _function_y ? _function_y->value(_t, dummy) : 0.0;
  Real z_t = _function_z ? _function_z->value(_t, dummy) : 0.0;

  // activate element when element is close to the point
  Real distance = (_current_elem->vertex_average() - Point(x_t, y_t, z_t)).norm();
  if (distance < _activate_distance)
    return true;
  else
    return false;
}
