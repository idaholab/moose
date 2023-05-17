//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedVectorAux.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", ParsedVectorAux);

InputParameters
ParsedVectorAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Sets a field vector variable value to the evaluation of a parsed expression.");

  params.addRequiredCustomTypeParam<std::string>(
      "expression_x",
      "FunctionExpression",
      "Parsed function expression to compute the x component");
  params.addCustomTypeParam<std::string>("expression_y",
                                         "0",
                                         "FunctionExpression",
                                         "Parsed function expression to compute the y component");
  if constexpr (LIBMESH_DIM >= 3)
    params.addCustomTypeParam<std::string>("expression_z",
                                           "0",
                                           "FunctionExpression",
                                           "Parsed function expression to compute the z component");
  params.addCoupledVar("coupled_variables", "Vector of coupled variable names");
  params.addCoupledVar("coupled_vector_variables", "Vector of coupled variable names");

  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression.");
  params.addParam<std::vector<std::vector<std::string>>>(
      "constant_names",
      std::vector<std::vector<std::string>>(),
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::vector<std::string>>>(
      "constant_expressions",
      std::vector<std::vector<std::string>>(),
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  return params;
}

ParsedVectorAux::ParsedVectorAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    FunctionParserUtils(parameters),
    _function({getParam<std::string>("expression_x"),
               getParam<std::string>("expression_y"),
               getParam<std::string>("expression_z")}),
    _nargs(coupledComponents("coupled_variables")),
    _n_vector_args(coupledComponents("coupled_vector_variables")),
    _args(coupledValues("coupled_variables")),
    _vector_args(coupledVectorValues("coupled_vector_variables")),
    _use_xyzt(getParam<bool>("use_xyzt"))
{
  _func_F.resize(LIBMESH_DIM);
  _function.resize(LIBMESH_DIM);

  if (getParam<std::vector<std::vector<std::string>>>("constant_names").size() !=
      getParam<std::vector<std::vector<std::string>>>("constant_expressions").size())
    paramError("constant_names", "Must be same length as constant_expressions");

  for (const auto i : make_range(Moose::dim))
  {
    // build variables argument
    std::string variables;

    // coupled regular field variables
    for (const auto i : make_range(_nargs))
      variables += (i == 0 ? "" : ",") + getFieldVar("coupled_variables", i)->name();

    // coupled vector field variables
    for (const auto i : make_range(_n_vector_args))
      variables +=
          (variables.empty() ? "" : ",") + getFieldVar("coupled_vector_variables", i)->name();

    // "system" variables
    const std::vector<std::string> xyzt = {"x", "y", "z", "t"};
    if (_use_xyzt)
      for (auto & v : xyzt)
        variables += (variables.empty() ? "" : ",") + v;

    // base function object
    _func_F[i] = std::make_shared<SymFunction>();

    // set FParser internal feature flags
    setParserFeatureFlags(_func_F[i]);

    // add the constant expressions
    if (getParam<std::vector<std::vector<std::string>>>("constant_names").size())
      addFParserConstants(
          _func_F[i],
          getParam<std::vector<std::vector<std::string>>>("constant_names")[i],
          getParam<std::vector<std::vector<std::string>>>("constant_expressions")[i]);

    // parse function
    if (_func_F[i]->Parse(_function[i], variables) >= 0)
      mooseError("Invalid function\n",
                 _function[i],
                 "\nin ParsedVectorAux '",
                 name(),
                 "' component ",
                 i == 0   ? "x"
                 : i == 1 ? "y"
                          : "z",
                 ".\n",
                 _func_F[i]->ErrorMsg());

    // optimize
    if (!_disable_fpoptimizer)
      _func_F[i]->Optimize();

    // just-in-time compile
    if (_enable_jit)
    {
      // let rank 0 do the JIT compilation first
      if (_communicator.rank() != 0)
        _communicator.barrier();

      _func_F[i]->JITCompile();

      // wait for ranks > 0 to catch up
      if (_communicator.rank() == 0)
        _communicator.barrier();
    }
  }
  // reserve storage for parameter passing buffer
  _func_params.resize(_nargs + _n_vector_args + (_use_xyzt ? 4 : 0));
}

RealVectorValue
ParsedVectorAux::computeValue()
{
  RealVectorValue value;
  for (const auto i : make_range(Moose::dim))
  {
    for (const auto j : make_range(_nargs))
      _func_params[j] = (*_args[j])[_qp];

    for (const auto j : make_range(_n_vector_args))
      _func_params[_nargs + j] = (*_vector_args[j])[_qp](i);

    if (_use_xyzt)
    {
      for (const auto j : make_range(Moose::dim))
        _func_params[_nargs + _n_vector_args + j] =
            isNodal() ? (*_current_node)(j) : _q_point[_qp](j);
      _func_params[_nargs + _n_vector_args + 3] = _t;
    }
    value(i) = evaluate(_func_F[i]);
  }
  return value;
}
