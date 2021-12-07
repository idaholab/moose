//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionElementIntegralUserObject.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionElementIntegralUserObject);

InputParameters
FunctionElementIntegralUserObject::validParams()
{
  InputParameters params = ElementIntegralUserObject::validParams();
  params.addClassDescription("Computes a volume integral of a function.");
  params.addRequiredParam<FunctionName>("function", "The function that this object operates on");
  return params;
}

FunctionElementIntegralUserObject::FunctionElementIntegralUserObject(
    const InputParameters & parameters)
  : ElementIntegralUserObject(parameters), _function(getFunction("function"))
{
}

Real
FunctionElementIntegralUserObject::computeQpIntegral()
{
  return _function.value(_t, _q_point[_qp]);
}
