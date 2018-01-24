//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseParsedGradFunction.h"
#include "MooseParsedFunctionWrapper.h"

template <>
InputParameters
validParams<MooseParsedGradFunction>()
{
  InputParameters params = validParams<Function>();
  params += validParams<MooseParsedFunctionBase>();
  params.addParam<std::string>("value", "0", "User defined function.");
  params.addParam<std::string>("grad_x", "0", "Partial with respect to x.");
  params.addParam<std::string>("grad_y", "0", "Partial with respect to y.");
  params.addParam<std::string>("grad_z", "0", "Partial with respect to z.");
  return params;
}

MooseParsedGradFunction::MooseParsedGradFunction(const InputParameters & parameters)
  : Function(parameters),
    MooseParsedFunctionBase(parameters),
    _value(verifyFunction(getParam<std::string>("value"))),
    _grad_value(verifyFunction(std::string("{") + getParam<std::string>("grad_x") + "}{" +
                               getParam<std::string>("grad_y") + "}{" +
                               getParam<std::string>("grad_z") + "}"))
{
}

MooseParsedGradFunction::~MooseParsedGradFunction() {}

Real
MooseParsedGradFunction::value(Real t, const Point & p)
{
  // Return a scalar value
  return _function_ptr->evaluate<Real>(t, p);
}

RealGradient
MooseParsedGradFunction::gradient(Real t, const Point & p)
{
  // Return gradient (RealGradient = RealVectorValue)
  return _grad_function_ptr->evaluate<RealVectorValue>(t, p);
}

RealVectorValue
MooseParsedGradFunction::vectorValue(Real /*t*/, const Point & /*p*/)
{
  mooseError("The vectorValue method is not defined in ParsedGradFunction");
}

void
MooseParsedGradFunction::initialSetup()
{
  THREAD_ID tid = 0;
  if (isParamValid("_tid"))
    tid = getParam<THREAD_ID>("_tid");

  if (!_function_ptr)
    _function_ptr =
        libmesh_make_unique<MooseParsedFunctionWrapper>(_pfb_feproblem, _value, _vars, _vals, tid);

  if (!_grad_function_ptr)
    _grad_function_ptr = libmesh_make_unique<MooseParsedFunctionWrapper>(
        _pfb_feproblem, _grad_value, _vars, _vals, tid);
}
