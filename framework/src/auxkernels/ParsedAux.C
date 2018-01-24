//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedAux.h"

template <>
InputParameters
validParams<ParsedAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params += validParams<FunctionParserUtils>();
  params.addClassDescription("Parsed function AuxKernel.");

  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "function expression");
  params.addCoupledVar("args", "coupled variables");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  return params;
}

ParsedAux::ParsedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("function")),
    _nargs(coupledComponents("args")),
    _args(_nargs)
{
  // build variables argument
  std::string variables;
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    variables += (i == 0 ? "" : ",") + getVar("args", i)->name();
    _args[i] = &coupledValue("args", i);
  }

  // base function object
  _func_F = ADFunctionPtr(std::make_shared<ADFunction>());

  // set FParser interneal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, variables) >= 0)
    mooseError(
        "Invalid function\n", _function, "\nin ParsedAux ", name(), ".\n", _func_F->ErrorMsg());

  // optimize
  if (!_disable_fpoptimizer)
    _func_F->Optimize();

  // just-in-time compile
  if (_enable_jit)
    _func_F->JITCompile();

  // reserve storage for parameter passing bufefr
  _func_params.resize(_nargs);
}

Real
ParsedAux::computeValue()
{
  for (unsigned int j = 0; j < _nargs; ++j)
    _func_params[j] = (*_args[j])[_qp];

  return evaluate(_func_F);
}
