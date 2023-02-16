//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedAux.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", ParsedAux);

InputParameters
ParsedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Sets a field variable value to the evaluation of a parsed expression.");

  params.addDeprecatedCustomTypeParam<std::string>("function",
                                                   "FunctionExpression",
                                                   "Parsed function expression to compute",
                                                   "'function' is deprecated, use 'expression'");
  // TODO Make required once deprecation is handled, see #19119
  params.addCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Parsed function expression to compute");
  params.addDeprecatedCoupledVar("args", "coupled_variables", "Vector of coupled variable names");
  params.addCoupledVar("coupled_variables", "Vector of coupled variable names");

  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression.");
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
    _function(getRenamedParam<std::string>("function", "expression")),
    _nargs(isCoupled("args") ? coupledComponents("args") : coupledComponents("coupled_variables")),
    _args(isCoupled("args") ? coupledValues("args") : coupledValues("coupled_variables")),
    _use_xyzt(getParam<bool>("use_xyzt"))
{
  // build variables argument
  std::string variables;

  // coupled field variables
  if (isCoupled("args"))
    for (std::size_t i = 0; i < _nargs; ++i)
    {
      auto * const field_var = getFieldVar("args", i);
      variables += (i == 0 ? "" : ",") + field_var->name();
    }
  else
    for (std::size_t i = 0; i < _nargs; ++i)
    {
      auto * const field_var = getFieldVar("coupled_variables", i);
      variables += (i == 0 ? "" : ",") + field_var->name();
    }

  // "system" variables
  const std::vector<std::string> xyzt = {"x", "y", "z", "t"};
  if (_use_xyzt)
    for (auto & v : xyzt)
      variables += (variables.empty() ? "" : ",") + v;

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
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

  // reserve storage for parameter passing buffer
  _func_params.resize(_nargs + (_use_xyzt ? 4 : 0));
}

Real
ParsedAux::computeValue()
{
  for (std::size_t j = 0; j < _nargs; ++j)
    _func_params[j] = (*_args[j])[_qp];

  if (_use_xyzt)
  {
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      _func_params[_nargs + j] = isNodal() ? (*_current_node)(j) : _q_point[_qp](j);
    _func_params[_nargs + 3] = _t;
  }

  return evaluate(_func_F);
}
