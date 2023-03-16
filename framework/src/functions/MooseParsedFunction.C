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
#include "FEProblemBase.h"

registerMooseObjectAliased("MooseApp", MooseParsedFunction, "ParsedFunction");
registerMooseObjectRenamed("MooseApp", ADParsedFunction, "02/03/2024 00:00", MooseParsedFunction);

InputParameters
MooseParsedFunction::validParams()
{
  InputParameters params = Function::validParams();
  params += MooseParsedFunctionBase::validParams();
  params.addDeprecatedCustomTypeParam<std::string>(
      "value", "FunctionExpression", "The user defined function.", "Use 'expression' instead.");
  // TODO Make required once deprecation is handled, see #19119
  params.addCustomTypeParam<std::string>(
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
  for (const auto i : index_range(_vars))
  {
    // Check for non-scalar variables.
    // First, see if the var is actually assigned to a proper scalar value
    if (_pfb_feproblem.hasVariable(_vars[i]) && _pfb_feproblem.hasVariable(_vals[i]))
    {
      // Then see if the var has the same name as a function or postprocessor
      if (!_pfb_feproblem.hasFunction(_vars[i]) &&
          !_pfb_feproblem.hasPostprocessorValueByName(_vars[i]))
        mooseError(
            "The only variables supported by ParsedFunction are scalar variables, and var '" +
            _vars[i] + "' is not scalar.");
    }
  }

  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (this->isParamValid("_tid"))
      tid = this->template getParam<THREAD_ID>("_tid");

    _function_ptr =
        std::make_unique<MooseParsedFunctionWrapper>(_pfb_feproblem, _value, _vars, _vals, tid);
  }
}
