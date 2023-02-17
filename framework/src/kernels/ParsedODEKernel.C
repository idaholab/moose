//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedODEKernel.h"

// MOOSE includes
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseApp.h"

#include "libmesh/fparser_ad.hh"

registerMooseObject("MooseApp", ParsedODEKernel);

InputParameters
ParsedODEKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription("Parsed expression ODE kernel.");

  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "function expression");
  params.deprecateParam("function", "expression", "02/07/2024");
  params.addCoupledVar("args", "Scalar variables coupled in the parsed expression.");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");
  params.addParam<std::vector<std::string>>("constant_names",
                                            "Vector of constants used in the parsed expression");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<std::vector<PostprocessorName>>(
      "postprocessors", "Vector of postprocessor names used in the function expression");

  return params;
}

ParsedODEKernel::ParsedODEKernel(const InputParameters & parameters)
  : ODEKernel(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("expression")),
    _nargs(isCoupledScalar("coupled_variables") ? coupledScalarComponents("coupled_variables") : 0),
    _args(_nargs),
    _arg_names(_nargs),
    _func_dFdarg(_nargs),
    _number_of_nl_variables(_sys.nVariables()),
    _arg_index(_number_of_nl_variables, -1)
{
  // build variables argument (start with variable the kernel is operating on)
  std::string variables = _var.name();

  // add additional coupled variables
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _arg_names[i] = getScalarVar("coupled_variables", i)->name();
    variables += "," + _arg_names[i];
    _args[i] = &coupledScalarValue("coupled_variables", i);

    // populate number -> arg index lookup table skipping aux variables
    unsigned int number = coupledScalar("coupled_variables", i);
    if (number < _number_of_nl_variables)
      _arg_index[number] = i;
  }

  // add postprocessors
  auto pp_names = getParam<std::vector<PostprocessorName>>("postprocessors");
  _pp.resize(pp_names.size());
  for (unsigned int i = 0; i < pp_names.size(); ++i)
  {
    variables += "," + pp_names[i];
    _pp[i] = &getPostprocessorValueByName(pp_names[i]);
  }

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser interneal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, variables) >= 0)
    mooseError("Invalid function\n",
               _function,
               "\nin ParsedODEKernel ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  // on-diagonal derivative
  _func_dFdu = std::make_shared<SymFunction>(*_func_F);

  if (_func_dFdu->AutoDiff(_var.name()) != -1)
    mooseError("Failed to take first derivative w.r.t. ", _var.name());

  // off-diagonal derivatives
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _func_dFdarg[i] = std::make_shared<SymFunction>(*_func_F);

    if (_func_dFdarg[i]->AutoDiff(_arg_names[i]) != -1)
      mooseError("Failed to take first derivative w.r.t. ", _arg_names[i]);
  }

  // optimize
  if (!_disable_fpoptimizer)
  {
    _func_F->Optimize();
    _func_dFdu->Optimize();
    for (unsigned int i = 0; i < _nargs; ++i)
      _func_dFdarg[i]->Optimize();
  }

  // just-in-time compile
  if (_enable_jit)
  {
    _func_F->JITCompile();
    _func_dFdu->JITCompile();
    for (unsigned int i = 0; i < _nargs; ++i)
      _func_dFdarg[i]->JITCompile();
  }

  // reserve storage for parameter passing buffer
  _func_params.resize(1 + _nargs + _pp.size());
}

void
ParsedODEKernel::updateParams()
{
  _func_params[0] = _u[_i];

  for (unsigned int j = 0; j < _nargs; ++j)
    _func_params[j + 1] = (*_args[j])[_i];
  for (unsigned int j = 0; j < _pp.size(); ++j)
    _func_params[j + 1 + _nargs] = *_pp[j];
}

Real
ParsedODEKernel::computeQpResidual()
{
  updateParams();
  return evaluate(_func_F);
}

Real
ParsedODEKernel::computeQpJacobian()
{
  updateParams();
  return evaluate(_func_dFdu);
}

Real
ParsedODEKernel::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  int i = _arg_index[jvar];
  if (i < 0)
    return 0.0;

  updateParams();
  return evaluate(_func_dFdarg[i]);
}
