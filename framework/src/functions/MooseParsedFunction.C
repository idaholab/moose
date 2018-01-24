//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"

// MOOSE includes
#include "InputParameters.h"
#include "MooseParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

template <>
InputParameters
validParams<MooseParsedFunction>()
{
  InputParameters params = validParams<Function>();
  params += validParams<MooseParsedFunctionBase>();
  params.addRequiredCustomTypeParam<std::string>(
      "value", "FunctionExpression", "The user defined function.");
  return params;
}

MooseParsedFunction::MooseParsedFunction(const InputParameters & parameters)
  : Function(parameters),
    MooseParsedFunctionBase(parameters),
    _value(verifyFunction(getParam<std::string>("value")))
{
}

Real
MooseParsedFunction::value(Real t, const Point & p)
{
  return _function_ptr->evaluate<Real>(t, p);
}

RealGradient
MooseParsedFunction::gradient(Real t, const Point & p)
{
  return _function_ptr->evaluateGradient(t, p);
}

Real
MooseParsedFunction::timeDerivative(Real t, const Point & p)
{
  return _function_ptr->evaluateDot(t, p);
}

RealVectorValue
MooseParsedFunction::vectorValue(Real /*t*/, const Point & /*p*/)
{
  mooseError("The vectorValue method is not defined in ParsedFunction");
}

void
MooseParsedFunction::initialSetup()
{
  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _function_ptr =
        libmesh_make_unique<MooseParsedFunctionWrapper>(_pfb_feproblem, _value, _vars, _vals, tid);
  }
}
