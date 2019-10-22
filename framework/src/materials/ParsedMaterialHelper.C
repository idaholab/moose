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
  std::string expression = function_expression;
  // build base function object
  _func_F = ADFunctionPtr(new ADFunction());

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

  // add further constants coming from default value coupling
  if (_map_mode == USE_PARAM_NAMES)
    for (const auto & acd : _arg_constant_defaults)
      if (!_func_F->AddConstant(acd, _pars.defaultCoupledValue(acd)))
        mooseError("Invalid constant name in parsed function object");

  _variable_names.resize(_nargs * 10);
  std::vector<std::string> temp_arg_names(_nargs * 10);
  // set variable names based on map_mode
  switch (_map_mode)
  {
    case USE_MOOSE_NAMES:
      for (unsigned int i = 0; i < _nargs; ++i)
        getVariableNames(expression, _arg_names[i], temp_arg_names, i);
      break;

    case USE_PARAM_NAMES:
<<<<<<< HEAD
      _variable_names.resize(_nargs * 10);
=======
>>>>>>> f29640c62d... Fix Error of Fix
      for (unsigned i = 0; i < _nargs; ++i)
      {
        if (_arg_param_numbers[i] < 0)
          getVariableNames(expression, _arg_param_names[i], temp_arg_names, i);
        else
          getVariableNames(expression,
                           _arg_param_names[i] + std::to_string(_arg_param_numbers[i]),
                           temp_arg_names,
                           i);
      }
      break;

    default:
      mooseError("Unnknown variable mapping mode");
  }

  _arg_names = temp_arg_names;
  _nargs *= 10;

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
  if (_func_F->Parse(expression, variables) >= 0)
    mooseError("Invalid function\n",
               expression,
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
ParsedMaterialHelper::getVariableNames(std::string & expression,
                                       const std::string & var_name,
                                       std::vector<std::string> & temp_arg_names,
                                       unsigned int index)
{
  temp_arg_names[10 * index] = _arg_names[index];
  temp_arg_names[10 * index + 1] = _arg_names[index] + "x";
  temp_arg_names[10 * index + 2] = _arg_names[index] + "y";
  temp_arg_names[10 * index + 3] = _arg_names[index] + "z";
  temp_arg_names[10 * index + 4] = _arg_names[index] + "xx";
  temp_arg_names[10 * index + 5] = _arg_names[index] + "xy";
  temp_arg_names[10 * index + 6] = _arg_names[index] + "xz";
  temp_arg_names[10 * index + 7] = _arg_names[index] + "yy";
  temp_arg_names[10 * index + 8] = _arg_names[index] + "yz";
  temp_arg_names[10 * index + 9] = _arg_names[index] + "zz";
  _variable_names[10 * index] = var_name;
  _variable_names[10 * index + 1] = var_name + "x";
  _variable_names[10 * index + 2] = var_name + "y";
  _variable_names[10 * index + 3] = var_name + "z";
  _variable_names[10 * index + 4] = var_name + "xx";
  _variable_names[10 * index + 5] = var_name + "xy";
  _variable_names[10 * index + 6] = var_name + "xz";
  _variable_names[10 * index + 7] = var_name + "yy";
  _variable_names[10 * index + 8] = var_name + "yz";
  _variable_names[10 * index + 9] = var_name + "zz";
  replaceDuplicates(expression, "yx");
  replaceDuplicates(expression, "zx");
  replaceDuplicates(expression, "zy");
}

void
ParsedMaterialHelper::replaceDuplicates(std::string & expression, const std::string & to_replace)
{

  std::size_t variable_location = expression.find(to_replace);
  char temp;
  while (variable_location != std::string::npos)
  {
    temp = expression[variable_location];
    expression[variable_location] = expression[variable_location + 1];
    expression[variable_location + 1] = temp;
    variable_location = expression.find(to_replace);
  }
}

void
ParsedMaterialHelper::functionsPostParse()
{
  functionsOptimize();

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : _mat_prop_descriptors)
    mpd.value();
}

void
ParsedMaterialHelper::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");
}

void
ParsedMaterialHelper::initQpStatefulProperties()
{
  if (_prop_F)
    (*_prop_F)[_qp] = 0.0;
}

void
ParsedMaterialHelper::computeQpProperties()
{
  std::vector<Real> _coupled_var_vals(10);
  Real a;

  // fill the parameter vector, apply tolerances
  for (unsigned int i = 0; i < _nargs / 10; ++i)
  {
    for (unsigned int j = 0; j < 10; j++)
    {
      getCoupledFuncParams(_coupled_var_vals, i);
      a = _coupled_var_vals[j];
      if (_tol[i] < 0.0)
        _func_params[10 * i + j] = a;
      else
        _func_params[10 * i + j] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
    }
  }

  // insert material property values
  auto nmat_props = _mat_prop_descriptors.size();
  for (MooseIndex(_mat_prop_descriptors) i = 0; i < nmat_props; ++i)
    _func_params[i + _nargs] = _mat_prop_descriptors[i].value()[_qp];

  // set function value
  if (_prop_F)
    (*_prop_F)[_qp] = evaluate(_func_F);
}

void
ParsedMaterialHelper::getCoupledFuncParams(std::vector<Real> & _coupled_var_vals, unsigned int i)
{
  _coupled_var_vals[0] = (*this->_args[i])[this->_qp];
  _coupled_var_vals[1] = ((*this->_grad_args[i])[this->_qp])(0);
  _coupled_var_vals[2] = ((*this->_grad_args[i])[this->_qp])(1);
  _coupled_var_vals[3] = ((*this->_grad_args[i])[this->_qp])(2);
  _coupled_var_vals[4] = ((*this->_second_args[i])[this->_qp])(0, 0);
  _coupled_var_vals[5] = ((*this->_second_args[i])[this->_qp])(0, 1);
  _coupled_var_vals[6] = ((*this->_second_args[i])[this->_qp])(0, 2);
  _coupled_var_vals[7] = ((*this->_second_args[i])[this->_qp])(1, 1);
  _coupled_var_vals[8] = ((*this->_second_args[i])[this->_qp])(1, 2);
  _coupled_var_vals[9] = ((*this->_second_args[i])[this->_qp])(2, 2);
}
