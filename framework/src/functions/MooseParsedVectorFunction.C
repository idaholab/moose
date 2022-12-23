//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseParsedVectorFunction.h"
#include "MooseParsedFunctionWrapper.h"

registerMooseObjectAliased("MooseApp", MooseParsedVectorFunction, "ParsedVectorFunction");

InputParameters
MooseParsedVectorFunction::validParams()
{
  InputParameters params = Function::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addClassDescription(
      "Return a vector component values based on string functions for each component.");
  params.addDeprecatedParam<std::string>(
      "value_x", "x-component of function.", "value_x is deprecated, use expression_x");
  params.addDeprecatedParam<std::string>(
      "value_y", "y-component of function.", "value_y is deprecated, use expression_y");
  params.addDeprecatedParam<std::string>(
      "value_z", "z-component of function.", "value_z is deprecated, use expression_z");
  params.addParam<std::string>("expression_x", "0", "x-component of function.");
  params.addParam<std::string>("expression_y", "0", "y-component of function.");
  params.addParam<std::string>("expression_z", "0", "z-component of function.");
  params.addParam<std::string>("curl_x", "0", "x-component of curl of function.");
  params.addParam<std::string>("curl_y", "0", "y-component of curl of function.");
  params.addParam<std::string>("curl_z", "0", "z-component of curl of function.");
  return params;
}

MooseParsedVectorFunction::MooseParsedVectorFunction(const InputParameters & parameters)
  : Function(parameters),
    MooseParsedFunctionBase(parameters),
    _vector_value(verifyFunction(std::string("{") +
                                 getRenamedParam<std::string>("value_x", "expression_x") + "}{" +
                                 getRenamedParam<std::string>("value_y", "expression_y") + "}{" +
                                 getRenamedParam<std::string>("value_z", "expression_z") + "}")),
    _curl_value(verifyFunction(std::string("{") + getParam<std::string>("curl_x") + "}{" +
                               getParam<std::string>("curl_y") + "}{" +
                               getParam<std::string>("curl_z") + "}"))
{
}

RealVectorValue
MooseParsedVectorFunction::vectorValue(Real t, const Point & p) const
{
  return _function_ptr->evaluate<RealVectorValue>(t, p);
}

RealVectorValue
MooseParsedVectorFunction::vectorCurl(Real t, const Point & p) const
{
  return _curl_function_ptr->evaluate<RealVectorValue>(t, p);
}

RealGradient
MooseParsedVectorFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("The gradient method is not defined in MooseParsedVectorFunction");
}

void
MooseParsedVectorFunction::initialSetup()
{
  THREAD_ID tid = 0;
  if (isParamValid("_tid"))
    tid = getParam<THREAD_ID>("_tid");

  if (!_function_ptr)
    _function_ptr = std::make_unique<MooseParsedFunctionWrapper>(
        _pfb_feproblem, _vector_value, _vars, _vals, tid);

  if (!_curl_function_ptr)
    _curl_function_ptr = std::make_unique<MooseParsedFunctionWrapper>(
        _pfb_feproblem, _curl_value, _vars, _vals, tid);
}
