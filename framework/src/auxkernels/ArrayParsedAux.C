//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayParsedAux.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", ArrayParsedAux);

InputParameters
ArrayParsedAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Sets field array variable values to the evaluation of a parsed expression.");

  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Parsed function expression to compute");
  params.addCoupledVar("coupled_variables", "Vector of coupled variable names.");
  params.addCoupledVar("coupled_array_variables", "Vector of coupled array variable names.");

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

ArrayParsedAux::ArrayParsedAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("expression")),
    _n_vars(coupledComponents("coupled_variables")),
    _n_array_vars(coupledComponents("coupled_array_variables")),
    _vars(coupledValues("coupled_variables")),
    _array_vars(coupledArrayValues("coupled_array_variables")),
    _use_xyzt(getParam<bool>("use_xyzt"))
{
  // build variables argument
  std::string variables;

  // coupled field variables
  for (const auto & i : make_range(_n_vars))
    variables += (i == 0 ? "" : ",") + getFieldVar("coupled_variables", i)->name();
  for (const auto & i : make_range(_n_array_vars))
  {
    const auto & var = *getArrayVar("coupled_array_variables", i);
    if (var.count() != _var.count())
      paramError("coupled_array_variables",
                 "The number of components in '",
                 _var.name(),
                 "' (",
                 _var.count(),
                 ") does not match the number of components in '",
                 var.name(),
                 "' (",
                 var.count(),
                 ").");
    variables += (i == 0 && _n_vars == 0 ? "" : ",") + var.name();
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
    mooseError("Invalid function\n",
               _function,
               "\nin ArrayParsedAux ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  // optimize
  if (!_disable_fpoptimizer)
    _func_F->Optimize();

  // just-in-time compile
  if (_enable_jit)
  {
    // let rank 0 do the JIT compilation first
    if (_communicator.rank() != 0)
      _communicator.barrier();

    _func_F->JITCompile();

    // wait for ranks > 0 to catch up
    if (_communicator.rank() == 0)
      _communicator.barrier();
  }

  // reserve storage for parameter passing buffer
  _func_params.resize(_n_vars + _n_array_vars + (_use_xyzt ? 4 : 0));
}

RealEigenVector
ArrayParsedAux::computeValue()
{
  // Returned value
  RealEigenVector val(_var.count());

  // Gather regular variable values
  for (const auto & i : make_range(_n_vars))
    _func_params[i] = (*_vars[i])[_qp];

  // Gather xyzt values
  if (_use_xyzt)
  {
    for (const auto j : make_range(Moose::dim))
      _func_params[_n_vars + _n_array_vars + j] =
          isNodal() ? (*_current_node)(j) : _q_point[_qp](j);
    _func_params[_n_vars + _n_array_vars + 3] = _t;
  }

  // Loop through each component
  for (const auto & c : make_range(_var.count()))
  {
    // Gather array variables
    for (const auto & i : make_range(_n_array_vars))
      _func_params[_n_vars + i] = (*_array_vars[i])[_qp](c);

    // Evaluate the parsed expression
    val(c) = evaluate(_func_F);
  }

  return val;
}
