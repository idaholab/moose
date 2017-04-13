/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ParsedMaterialHelper.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ParsedMaterialHelper>()
{
  InputParameters params = validParams<FunctionMaterialBase>();
  params += validParams<FunctionParserUtils>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

ParsedMaterialHelper::ParsedMaterialHelper(const InputParameters & parameters,
                                           VariableNameMappingMode map_mode)
  : FunctionMaterialBase(parameters),
    FunctionParserUtils(parameters),
    _variable_names(_nargs),
    _mat_prop_descriptors(0),
    _tol(0),
    _map_mode(map_mode)
{
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression)
{
  std::vector<std::string> empty_string_vector;
  functionParse(function_expression, empty_string_vector, empty_string_vector);
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression,
                                    const std::vector<std::string> & constant_names,
                                    const std::vector<std::string> & constant_expressions)
{
  std::vector<std::string> empty_string_vector;
  std::vector<Real> empty_real_vector;
  functionParse(function_expression,
                constant_names,
                constant_expressions,
                empty_string_vector,
                empty_string_vector,
                empty_real_vector);
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression,
                                    const std::vector<std::string> & constant_names,
                                    const std::vector<std::string> & constant_expressions,
                                    const std::vector<std::string> & mat_prop_expressions,
                                    const std::vector<std::string> & tol_names,
                                    const std::vector<Real> & tol_values)
{
  // build base function object
  _func_F = ADFunctionPtr(new ADFunction());

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

  // add further constants coming from default value coupling
  if (_map_mode == USE_PARAM_NAMES)
    for (std::vector<std::string>::iterator it = _arg_constant_defaults.begin();
         it != _arg_constant_defaults.end();
         ++it)
      if (!_func_F->AddConstant(*it, _pars.defaultCoupledValue(*it)))
        mooseError("Invalid constant name in parsed function object");

  // set variable names based on map_mode
  switch (_map_mode)
  {
    case USE_MOOSE_NAMES:
      for (unsigned i = 0; i < _nargs; ++i)
        _variable_names[i] = _arg_names[i];
      break;

    case USE_PARAM_NAMES:
      // we do not allow vector coupling in this mode
      if (!_mapping_is_unique)
        mooseError("Derivative parsed materials must couple exactly one non-linear variable per "
                   "coupled variable input parameter.");

      for (unsigned i = 0; i < _nargs; ++i)
        _variable_names[i] = _arg_param_names[i];
      break;

    default:
      mooseError("Unnknown variable mapping mode.");
  }

  // tolerance vectors
  if (tol_names.size() != tol_values.size())
    mooseError("The parameter vectors tol_names and tol_values must have equal length.");

  // set tolerances
  _tol.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _tol[i] = -1.0;

    // for every argument look throug the entire tolerance vector to find a match
    for (unsigned int j = 0; j < tol_names.size(); ++j)
      if (_variable_names[i] == tol_names[j])
      {
        _tol[i] = tol_values[j];
        break;
      }
  }

  // build 'variables' argument for fparser
  std::string variables;
  for (unsigned i = 0; i < _nargs; ++i)
    variables += "," + _variable_names[i];

  // get all material properties
  unsigned int nmat_props = mat_prop_expressions.size();
  _mat_prop_descriptors.resize(nmat_props);
  for (unsigned int i = 0; i < nmat_props; ++i)
  {
    // parse the material property parameter entry into a FunctionMaterialPropertyDescriptor
    _mat_prop_descriptors[i] = FunctionMaterialPropertyDescriptor(mat_prop_expressions[i], this);

    // get the fparser symbol name for the new material property
    variables += "," + _mat_prop_descriptors[i].getSymbolName();
  }

  // erase leading comma
  variables.erase(0, 1);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
    mooseError("Invalid function\n",
               function_expression,
               '\n',
               variables,
               "\nin ParsedMaterialHelper.\n",
               _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(_nargs + nmat_props);

  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

void
ParsedMaterialHelper::functionsPostParse()
{
  functionsOptimize();

  // force a value update to get the property at least once and register it for the dependencies
  unsigned int nmat_props = _mat_prop_descriptors.size();
  for (unsigned int i = 0; i < nmat_props; ++i)
    _mat_prop_descriptors[i].value();
}

void
ParsedMaterialHelper::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");
}

void
ParsedMaterialHelper::computeProperties()
{
  Real a;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (unsigned int i = 0; i < _nargs; ++i)
    {
      if (_tol[i] < 0.0)
        _func_params[i] = (*_args[i])[_qp];
      else
      {
        a = (*_args[i])[_qp];
        _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
      }
    }

    // insert material property values
    unsigned int nmat_props = _mat_prop_descriptors.size();
    for (unsigned int i = 0; i < nmat_props; ++i)
      _func_params[i + _nargs] = _mat_prop_descriptors[i].value()[_qp];

    // TODO: computeQpProperties()

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = evaluate(_func_F);
  }
}
