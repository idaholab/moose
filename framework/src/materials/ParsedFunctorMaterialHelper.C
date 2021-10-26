//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedFunctorMaterialHelper.h"

#include "libmesh/quadrature.h"

template <bool is_ad>
InputParameters
ParsedFunctorMaterialHelper<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += FunctionParserUtils<is_ad>::validParams();
  params.addClassDescription("Parsed Function Material.");
  params.addRequiredParam<std::string>("prop_name", "Name of the material property");
  params.addParam<bool>("error_on_missing_material_properties",
                        true,
                        "Throw an error if any explicitly requested material property does not "
                        "exist. Otherwise assume it to be zero.");
  return params;
}

template <bool is_ad>
ParsedFunctorMaterialHelper<is_ad>::ParsedFunctorMaterialHelper(const InputParameters & parameters,
                                                                VariableNameMappingMode map_mode)
  : FunctorMaterial(parameters),
    FunctionParserUtils<is_ad>(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _property(declareFunctorProperty<GenericReal<is_ad>>("prop_name")),
    _mat_prop_descriptors(0),
    _tol(0),
    _map_mode(map_mode),
    _error_on_missing_material_properties(
        this->template getParam<bool>("error_on_missing_material_properties"))
{


////////////////////////////////

  // fetch names and numbers of all coupled variables
  _mapping_is_unique = true;
  for (std::set<std::string>::const_iterator it = _pars.coupledVarsBegin();
       it != _pars.coupledVarsEnd();
       ++it)
  {
    // find the variable in the list of coupled variables
    auto vars = _coupled_vars.find(*it);

    // no MOOSE variable was provided for this coupling, add to a list of variables set to constant
    // default values
    if (vars == _coupled_vars.end())
    {
      if (_pars.hasDefaultCoupledValue(*it))
        _arg_constant_defaults.push_back(*it);
      continue;
    }

    // check if we have a 1:1 mapping between parameters and variables
    if (vars->second.size() != 1)
      _mapping_is_unique = false;

    // iterate over all components
    for (unsigned int j = 0; j < vars->second.size(); ++j)
    {
      // make sure each nonlinear variable is coupled in only once
      if (std::find(_arg_names.begin(), _arg_names.end(), vars->second[j]->name()) !=
          _arg_names.end())
        mooseError("A nonlinear variable can only be coupled in once.");

      // insert the map values
      // unsigned int number = vars->second[j]->number();
      unsigned int number = coupled(*it, j);
      _arg_names.push_back(vars->second[j]->name());
      _arg_numbers.push_back(number);
      _arg_param_names.push_back(*it);
      if (_mapping_is_unique)
        _arg_param_numbers.push_back(-1);
      else
        _arg_param_numbers.push_back(j);

      // populate number -> arg index lookup table
      unsigned int idx = libMeshVarNumberRemap(number);
      if (idx >= _arg_index.size())
        _arg_index.resize(idx + 1, -1);

      _arg_index[idx] = _variables.size();

      // get variable value
      // _variables.push_back(getFunctor<ADReal>(vars->second[j]->name()));
    }
  }

  _nargs = _arg_names.size();
  _symbol_names.resize(_nargs);

//////////////////////////////////////////////
}

template <bool is_ad>
void
ParsedFunctorMaterialHelper<is_ad>::functionParse(const std::string & function_expression)
{
  const std::vector<std::string> empty_string_vector;
  functionParse(function_expression, empty_string_vector, empty_string_vector);
}

template <bool is_ad>
void
ParsedFunctorMaterialHelper<is_ad>::functionParse(const std::string & function_expression,
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
ParsedFunctorMaterialHelper<is_ad>::functionParse(const std::string & function_expression,
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
ParsedFunctorMaterialHelper<is_ad>::functionParse(
    const std::string & function_expression,
    const std::vector<std::string> & constant_names,
    const std::vector<std::string> & constant_expressions,
    const std::vector<std::string> & mat_prop_expressions,
    const std::vector<PostprocessorName> & postprocessor_names,
    const std::vector<std::string> & tol_names,
    const std::vector<Real> & tol_values)
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
      if (!_func_F->AddConstant(acd, _pars.defaultCoupledValue(acd)))
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
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _tol[i] = -1.0;

    // for every argument look through the entire tolerance vector to find a match
    for (MooseIndex(tol_names) j = 0; j < tol_names.size(); ++j)
      if (_symbol_names[i] == tol_names[j])
      {
        _tol[i] = tol_values[j];
        break;
      }
  }

  // get all material properties
  unsigned int nmat_props = mat_prop_expressions.size();
  _mat_prop_descriptors.resize(nmat_props);
  for (unsigned int i = 0; i < nmat_props; ++i)
  {
    // parse the material property parameter entry into a FunctionMaterialPropertyDescriptor
    _mat_prop_descriptors[i] = FunctionMaterialPropertyDescriptor<is_ad>(
        mat_prop_expressions[i], this, _error_on_missing_material_properties);

    // get the fparser symbol name for the new material property
    _symbol_names.push_back(_mat_prop_descriptors[i].getSymbolName());
  }

  // get all coupled postprocessors
  for (const auto & pp : postprocessor_names)
  {
    _postprocessor_values.push_back(&this->getPostprocessorValueByName(pp));
    _symbol_names.push_back(pp);
  }

  // build 'variables' argument for fparser
  std::string variables = Moose::stringify(_symbol_names);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
    mooseError("Invalid function\n",
               function_expression,
               '\n',
               variables,
               "\nin ParsedFunctorMaterialHelper.\n",
               _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(_nargs + nmat_props + _postprocessor_values.size());

  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

template <bool is_ad>
void
ParsedFunctorMaterialHelper<is_ad>::functionsPostParse()
{
  functionsOptimize();

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : _mat_prop_descriptors)
    mpd.value();
}

template <>
void
ParsedFunctorMaterialHelper<false>::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");
}

template <>
void
ParsedFunctorMaterialHelper<true>::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (!_enable_jit || !_func_F->JITCompile())
    mooseError("ADParsedMaterials require JIT compilation to be enabled and working.");
}

// explicit instantiation
template class ParsedFunctorMaterialHelper<false>;
template class ParsedFunctorMaterialHelper<true>;
