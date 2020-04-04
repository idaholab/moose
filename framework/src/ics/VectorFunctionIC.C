//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFunctionIC.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", VectorFunctionIC);

InputParameters
VectorFunctionIC::validParams()
{
  InputParameters params = VectorInitialCondition::validParams();
  params.addParam<FunctionName>("function",
                                "The initial condition vector function. This cannot be supplied "
                                "with the component parameters.");
  params.addParam<FunctionName>(
      "function_x", "0", "A function that describes the x-component of the initial condition");
  params.addParam<FunctionName>(
      "function_y", "0", "A function that describes the y-component of the initial condition");
  params.addParam<FunctionName>(
      "function_z", "0", "A function that describes the z-component of the initial condition");
  params.addClassDescription(
      "Sets component values for a vector field variable based on a vector function.");
  return params;
}

VectorFunctionIC::VectorFunctionIC(const InputParameters & parameters)
  : VectorInitialCondition(parameters),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z"))
{
  if (_function && parameters.isParamSetByUser("function_x"))
    paramError("function_x", "The 'function' and 'function_x' parameters cannot both be set.");
  if (_function && parameters.isParamSetByUser("function_y"))
    paramError("function_y", "The 'function' and 'function_y' parameters cannot both be set.");
  if (_function && parameters.isParamSetByUser("function_z"))
    paramError("function_z", "The 'function' and 'function_z' parameters cannot both be set.");
}

RealVectorValue
VectorFunctionIC::value(const Point & p)
{
  if (_function)
    return _function->vectorValue(_t, p);
  else
    return RealVectorValue(
        _function_x.value(_t, p), _function_y.value(_t, p), _function_z.value(_t, p));
}
