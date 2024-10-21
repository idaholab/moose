//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedMaterialHelper.h"

#include "libmesh/quadrature.h"
#include "Conversion.h"

template <bool is_ad>
InputParameters
ParsedMaterialHelper<is_ad>::validParams()
{
  InputParameters params = FunctionMaterialBase<is_ad>::validParams();
  params += FunctionParserUtils<is_ad>::validParams();
  params.addClassDescription("Parsed Function Material.");
  params.addParam<bool>("error_on_missing_material_properties",
                        true,
                        "Throw an error if any explicitly requested material property does not "
                        "exist. Otherwise assume it to be zero.");
  MultiMooseEnum extra_symbols("x y z t dt");
  params.addParam<MultiMooseEnum>(
      "extra_symbols",
      extra_symbols,
      "Special symbols, like point coordinates, time, and timestep size.");
  params.addParam<std::vector<MaterialName>>(
      "upstream_materials",
      std::vector<MaterialName>(),
      "List of upstream material properties that must be evaluated when compute=false");
  return params;
}

template <bool is_ad>
ParsedMaterialHelper<is_ad>::ParsedMaterialHelper(const InputParameters & parameters,
                                                  VariableNameMappingMode map_mode)
  : FunctionMaterialBase<is_ad>(parameters),
    FunctionParserUtils<is_ad>(parameters),
    _symbol_names(_nargs),
    _extra_symbols(this->template getParam<MultiMooseEnum>("extra_symbols")
                       .template getSetValueIDs<ExtraSymbols>()),
    _tol(0),
    _map_mode(map_mode),
    _upstream_mat_names(this->template getParam<std::vector<MaterialName>>("upstream_materials")),
    _error_on_missing_material_properties(
        this->template getParam<bool>("error_on_missing_material_properties"))
{
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::insertReservedNames(std::set<std::string> & reserved_names)
{
  for (const auto symbol : _extra_symbols)
    switch (symbol)
    {
      case ExtraSymbols::x:
        reserved_names.insert("x");
        break;
      case ExtraSymbols::y:
        reserved_names.insert("y");
        break;
      case ExtraSymbols::z:
        reserved_names.insert("z");
        break;
      case ExtraSymbols::t:
        reserved_names.insert("t");
        break;
      case ExtraSymbols::dt:
        reserved_names.insert("dt");
        break;
    };
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionParse(const std::string & function_expression)
{
  const std::vector<std::string> empty_string_vector;
  functionParse(function_expression, empty_string_vector, empty_string_vector);
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionParse(const std::string & function_expression,
                                           const std::vector<std::string> & constant_names,
                                           const std::vector<std::string> & constant_expressions)
{
  const std::vector<std::string> empty_string_vector;
  const std::vector<Real> empty_real_vector;
  functionParse(function_expression,
                constant_names,
                constant_expressions,
                empty_string_vector,
                empty_string_vector,
                empty_real_vector);
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionParse(const std::string & function_expression,
                                           const std::vector<std::string> & constant_names,
                                           const std::vector<std::string> & constant_expressions,
                                           const std::vector<std::string> & mat_prop_expressions,
                                           const std::vector<std::string> & tol_names,
                                           const std::vector<Real> & tol_values)
{
  const std::vector<PostprocessorName> empty_pp_name_vector;
  functionParse(function_expression,
                constant_names,
                constant_expressions,
                mat_prop_expressions,
                empty_pp_name_vector,
                tol_names,
                tol_values);
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionParse(
    const std::string & function_expression,
    const std::vector<std::string> & constant_names,
    const std::vector<std::string> & constant_expressions,
    const std::vector<std::string> & mat_prop_expressions,
    const std::vector<PostprocessorName> & postprocessor_names,
    const std::vector<std::string> & tol_names,
    const std::vector<Real> & tol_values)
{
  const std::vector<MooseFunctorName> empty_functor_vector;
  const std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                constant_names,
                constant_expressions,
                mat_prop_expressions,
                postprocessor_names,
                tol_names,
                tol_values,
                empty_functor_vector,
                empty_string_vector);
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionParse(
    const std::string & function_expression,
    const std::vector<std::string> & constant_names,
    const std::vector<std::string> & constant_expressions,
    const std::vector<std::string> & mat_prop_expressions,
    const std::vector<PostprocessorName> & postprocessor_names,
    const std::vector<std::string> & tol_names,
    const std::vector<Real> & tol_values,
    const std::vector<MooseFunctorName> & functor_names,
    const std::vector<std::string> & functor_symbols)
{
  // build base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

  // add further constants coming from default value coupling
  if (_map_mode == VariableNameMappingMode::USE_PARAM_NAMES)
    for (const auto & acd : _arg_constant_defaults)
      if (!_func_F->AddConstant(acd, this->_pars.defaultCoupledValue(acd)))
        mooseError("Invalid constant name in parsed function object");

  // set variable names based on map_mode
  switch (_map_mode)
  {
    case VariableNameMappingMode::USE_MOOSE_NAMES:
      for (unsigned int i = 0; i < _nargs; ++i)
        _symbol_names[i] = _arg_names[i];
      break;

    case VariableNameMappingMode::USE_PARAM_NAMES:
      for (unsigned i = 0; i < _nargs; ++i)
      {
        if (_arg_param_numbers[i] < 0)
          _symbol_names[i] = _arg_param_names[i];
        else
          _symbol_names[i] = _arg_param_names[i] + std::to_string(_arg_param_numbers[i]);
      }
      break;

    default:
      mooseError("Unknown variable mapping mode.");
  }

  // tolerance vectors
  if (tol_names.size() != tol_values.size())
    mooseError("The parameter vectors tol_names and tol_values must have equal length.");

  // set tolerances
  _tol.resize(_nargs);
  for (const auto i : make_range(_nargs))
  {
    _tol[i] = -1.0;

    // for every argument look through the entire tolerance vector to find a match
    for (const auto j : index_range(tol_names))
      if (_symbol_names[i] == tol_names[j])
      {
        _tol[i] = tol_values[j];
        break;
      }
  }

  // get all material properties
  unsigned int nmat_props = mat_prop_expressions.size();
  for (const auto i : make_range(nmat_props))
  {
    // parse the material property parameter entry into a FunctionMaterialPropertyDescriptor
    _mat_prop_descriptors.emplace_back(
        mat_prop_expressions[i], this, _error_on_missing_material_properties);

    // get the fparser symbol name for the new material property
    _symbol_names.push_back(_mat_prop_descriptors.back().getSymbolName());
  }

  // get all coupled postprocessors
  for (const auto & pp : postprocessor_names)
  {
    _postprocessor_values.push_back(&this->getPostprocessorValueByName(pp));
    _symbol_names.push_back(pp);
  }

  // get all extra symbols
  for (const auto symbol : _extra_symbols)
    switch (symbol)
    {
      case ExtraSymbols::x:
        _symbol_names.push_back("x");
        break;
      case ExtraSymbols::y:
        _symbol_names.push_back("y");
        break;
      case ExtraSymbols::z:
        _symbol_names.push_back("z");
        break;
      case ExtraSymbols::t:
        _symbol_names.push_back("t");
        break;
      case ExtraSymbols::dt:
        _symbol_names.push_back("dt");
        break;
    }

  // get all functors
  if (!functor_symbols.empty() && functor_symbols.size() != functor_names.size())
    mooseError("The parameter vector functor_symbols must be of same length as functor_names, if "
               "not empty.");
  _functors.resize(functor_names.size());
  for (const auto i : index_range(functor_names))
  {
    if (functor_symbols.empty())
    {
      auto functor_name = functor_names[i];
      _symbol_names.push_back(functor_name);
      _functors[i] = &FunctorInterface::getFunctor<Real>(functor_name);
    }
    else
    {
      auto functor_name = functor_names[i];
      auto symbol_name = functor_symbols[i];
      _symbol_names.push_back(symbol_name);
      _functors[i] = &FunctorInterface::getFunctor<Real>(functor_name);
    }
  }

  // build 'variables' argument for fparser
  std::string variables = Moose::stringify(_symbol_names);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
    mooseError("Invalid function\n",
               function_expression,
               '\n',
               variables,
               "\nin ParsedMaterialHelper.\n",
               _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(_nargs + nmat_props + _postprocessor_values.size() + _extra_symbols.size() +
                      functor_names.size());

  // perform next steps (either optimize or take derivatives and then optimize)

  // let rank 0 do the work first to populate caches
  if (_communicator.rank() != 0)
    _communicator.barrier();

  functionsPostParse();

  // wait for ranks > 0 to catch up
  if (_communicator.rank() == 0)
    _communicator.barrier();
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::functionsPostParse()
{
  functionsOptimize(_func_F);

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : _mat_prop_descriptors)
    mpd.value();
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::initialSetup()
{
  _upstream_mat.resize(_upstream_mat_names.size());
  for (const auto i : make_range(_upstream_mat_names.size()))
    _upstream_mat[i] = &this->getMaterialByName(_upstream_mat_names[i]);
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
ParsedMaterialHelper<is_ad>::computeQpProperties()
{
  if (!(this->_compute))
  {
    for (const auto i : make_range(_upstream_mat_names.size()))
      _upstream_mat[i]->computePropertiesAtQp(_qp);
  }

  // fill the parameter vector, apply tolerances
  for (const auto i : make_range(_nargs))
  {
    if (_tol[i] < 0.0)
      _func_params[i] = (*_args[i])[_qp];
    else
    {
      auto a = (*_args[i])[_qp];
      _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
    }
  }
  auto offset = _nargs;

  // insert material property values
  for (const auto i : index_range(_mat_prop_descriptors))
    _func_params[i + offset] = _mat_prop_descriptors[i].value(_qp);
  offset += _mat_prop_descriptors.size();

  // insert postprocessor values
  auto npps = _postprocessor_values.size();
  for (MooseIndex(_postprocessor_values) i = 0; i < npps; ++i)
    _func_params[i + offset] = *_postprocessor_values[i];
  offset += _postprocessor_values.size();

  // insert extra symbol values
  for (const auto i : index_range(_extra_symbols))
  {
    const auto j = offset + i;
    switch (_extra_symbols[i])
    {
      case ExtraSymbols::x:
        _func_params[j] = _q_point[_qp](0);
        break;
      case ExtraSymbols::y:
        _func_params[j] = _q_point[_qp](1);
        break;
      case ExtraSymbols::z:
        _func_params[j] = _q_point[_qp](2);
        break;
      case ExtraSymbols::t:
        _func_params[j] = _t;
        break;
      case ExtraSymbols::dt:
        _func_params[j] = _dt;
        break;
    }
  }
  offset += _extra_symbols.size();

  // insert functor values
  const auto & state = TransientInterface::determineState();
  const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  for (const auto i : index_range(_functors))
    _func_params[offset + i] = (*_functors[i])(qp_arg, state);

  // set function value
  if (_prop_F)
    (*_prop_F)[_qp] = evaluate(_func_F, _name);
}

// explicit instantiation
template class ParsedMaterialHelper<false>;
template class ParsedMaterialHelper<true>;
