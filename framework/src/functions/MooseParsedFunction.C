//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "FEProblemBase.h"

registerMooseObjectAliased("MooseApp", MooseParsedFunction, "ParsedFunction");

InputParameters
MooseParsedFunction::validParams()
{
  InputParameters params = Function::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "The user defined function.");
  params.addClassDescription("Function created by parsing a string");
  return params;
}

MooseParsedFunction::MooseParsedFunction(const InputParameters & parameters)
  : Function(parameters),
    MooseParsedFunctionBase(parameters),
    _value(verifyFunction(this->template getRenamedParam<std::string>("value", "expression")))
{
}

Real
MooseParsedFunction::value(Real t, const Point & p) const
{
  mooseAssert(_function_ptr, "ParsedFunction should have been initialized");
  return _function_ptr->evaluate<Real>(t, p);
}

RealGradient
MooseParsedFunction::gradient(Real t, const Point & p) const
{
  mooseAssert(_function_ptr, "ParsedFunction should have been initialized");
  return _function_ptr->evaluateGradient(t, p);
}

Real
MooseParsedFunction::timeDerivative(Real t, const Point & p) const
{
  mooseAssert(_function_ptr, "ParsedFunction should have been initialized");
  return _function_ptr->evaluateDot(t, p);
}

RealVectorValue
MooseParsedFunction::vectorValue(Real /*t*/, const Point & /*p*/) const
{
  mooseError("The vectorValue method is not defined in ParsedFunction");
}

void
MooseParsedFunction::initialSetup()
{
  // Check for non-scalar variables.
  for (const auto i : index_range(_vars))
    if (_pfb_feproblem.hasVariable(_vals[i]) && !_pfb_feproblem.hasScalarVariable(_vals[i]) &&
        !_pfb_feproblem.hasFunction(_vals[i]) &&
        !_pfb_feproblem.hasPostprocessorValueByName(_vals[i]))
      mooseError("The only variables supported by ParsedFunction are scalar variables, and var '" +
                 _vals[i] + "' is not scalar.");

  if (!_function_ptr)
  {
    THREAD_ID tid = this->isParamValid("_tid") ? this->template getParam<THREAD_ID>("_tid") : 0;

    _function_ptr =
        std::make_unique<MooseParsedFunctionWrapper>(_pfb_feproblem, _value, _vars, _vals, tid);
  }
}
