//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedAux.h"

registerMooseObject("MooseApp", ParsedAux);

InputParameters
ParsedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Sets a field variable value to the evaluation of a parsed expression.");

  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "Parsed function expression to compute");
  params.deprecateParam("function", "expression", "02/07/2024");
  params.addCoupledVar("args", "Vector of coupled variable names");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");

  params.addParam<std::vector<MaterialPropertyName>>(
      "material_properties", {}, "Material properties (Real-valued) in the expression");
  params.addParam<std::vector<MaterialPropertyName>>(
      "ad_material_properties", {}, "AD material properties (ADReal-valued) in the expression");

  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression.");
  params.addParam<std::vector<std::string>>(
      "constant_names",
      {},
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<std::vector<MooseFunctorName>>(
      "functor_names", {}, "Functors to use in the parsed expression");
  params.addParam<std::vector<std::string>>(
      "functor_symbols",
      {},
      "Symbolic name to use for each functor in 'functor_names' in the parsed expression. If not "
      "provided, then the actual functor names will be used in the parsed expression.");

  return params;
}

ParsedAux::ParsedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("expression")),
    _nargs(coupledComponents("coupled_variables")),
    _args(coupledValues("coupled_variables")),
    _matprop_names(getParam<std::vector<MaterialPropertyName>>("material_properties")),
    _ad_matprop_names(getParam<std::vector<MaterialPropertyName>>("ad_material_properties")),
    _n_matprops(_matprop_names.size()),
    _n_ad_matprops(_ad_matprop_names.size()),
    _use_xyzt(getParam<bool>("use_xyzt")),
    _xyzt({"x", "y", "z", "t"}),
    _functor_names(getParam<std::vector<MooseFunctorName>>("functor_names")),
    _n_functors(_functor_names.size()),
    _functor_symbols(getParam<std::vector<std::string>>("functor_symbols"))
{

  for (const auto i : make_range(_nargs))
    _coupled_variable_names.push_back(getFieldVar("coupled_variables", i)->name());

  // sanity checks
  if (!_functor_symbols.empty() && (_functor_symbols.size() != _n_functors))
    paramError("functor_symbols", "functor_symbols must be the same length as functor_names.");

  validateFunctorSymbols();
  validateFunctorNames();

  // build variables argument
  std::string variables;

  // coupled field variables
  for (const auto i : index_range(_coupled_variable_names))
    variables += (i == 0 ? "" : ",") + _coupled_variable_names[i];

  // adding functors to the expression
  if (_functor_symbols.size())
    for (const auto & symbol : _functor_symbols)
      variables += (variables.empty() ? "" : ",") + symbol;
  else
    for (const auto & name : _functor_names)
      variables += (variables.empty() ? "" : ",") + name;

  // material properties
  for (const auto & matprop : _matprop_names)
    variables += (variables.empty() ? "" : ",") + matprop;
  for (const auto & matprop : _ad_matprop_names)
    variables += (variables.empty() ? "" : ",") + matprop;

  // positions and time
  if (_use_xyzt)
    for (auto & v : _xyzt)
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
  _func_params.resize(_nargs + _n_functors + _n_matprops + _n_ad_matprops + (_use_xyzt ? 4 : 0));

  // keep pointers to the material properties
  for (const auto & name : _matprop_names)
    _matprops.push_back(&getMaterialProperty<Real>(name));
  for (const auto & name : _ad_matprop_names)
    _ad_matprops.push_back(&getADMaterialProperty<Real>(name));

  // keep pointers to the functors
  for (const auto & name : _functor_names)
    _functors.push_back(&getFunctor<Real>(name));
}

Real
ParsedAux::computeValue()
{
  // Variables
  for (const auto j : make_range(_nargs))
    _func_params[j] = (*_args[j])[_qp];

  // Functors
  const auto & state = determineState();
  if (isNodal())
  {
    const Moose::NodeArg node_arg = {_current_node, Moose::INVALID_BLOCK_ID};
    for (const auto i : index_range(_functors))
      _func_params[_nargs + i] = (*_functors[i])(node_arg, state);
  }
  else
  {
    const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
    for (const auto i : index_range(_functors))
      _func_params[_nargs + i] = (*_functors[i])(qp_arg, state);
  }

  // Material properties
  for (const auto j : make_range(_n_matprops))
    _func_params[_nargs + _n_functors + j] = (*_matprops[j])[_qp];
  for (const auto j : make_range(_n_ad_matprops))
    _func_params[_nargs + _n_functors + _n_matprops + j] = (*_ad_matprops[j])[_qp].value();

  // Positions and time
  if (_use_xyzt)
  {
    for (const auto j : make_range(LIBMESH_DIM))
      _func_params[_nargs + _n_functors + _n_matprops + _n_ad_matprops + j] =
          isNodal() ? (*_current_node)(j) : _q_point[_qp](j);
    _func_params[_nargs + _n_functors + _n_matprops + _n_ad_matprops + 3] = _t;
  }

  return evaluate(_func_F);
}

void
ParsedAux::validateFunctorSymbols()
{
  validateGenericVectorNames(_functor_symbols, "functor_symbols");
}

void
ParsedAux::validateFunctorNames()
{
  validateGenericVectorNames(_functor_names, "functor_names");
}
