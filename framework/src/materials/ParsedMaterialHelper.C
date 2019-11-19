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
  functionParse(function_expression, constant_names, constant_expressions, empty_string_vector);
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression,
                                    const std::vector<std::string> & constant_names,
                                    const std::vector<std::string> & constant_expressions,
                                    const std::vector<std::string> & mat_prop_expressions)
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

  // set variable names based on map_mode
  switch (_map_mode)
  {
    case USE_MOOSE_NAMES:
      // replace duplicate derivative terms (i.e. D[c,xy] == D[c,yx])
      for (unsigned int i = 0; i < _nargs; ++i)
        replaceDuplicates(expression, _arg_names[i]);
      // get variable names from FParser
      _func_F->ParseAndDeduceVariables(expression, _variable_names, false);
      _nargs = _variable_names.size();
      variable_info.resize(_nargs);

      // get our coupled values
      for (unsigned int i = 0; i < _nargs; ++i)
        setCoupledValues(_arg_names[i], _nargs);
      break;

    case USE_PARAM_NAMES:
<<<<<<< HEAD
<<<<<<< HEAD
      _variable_names.resize(_nargs * 10);
=======
>>>>>>> f29640c62d... Fix Error of Fix
=======
      // replace duplicate derivative terms (i.e. D[c,xy] == D[c,yx])
>>>>>>> 6b9de55e13... Allowed for FParser to deduce the variables used, and only allocating memory for the ones that were used #14366
      for (unsigned i = 0; i < _nargs; ++i)
      {
        if (_arg_param_numbers[i] < 0)
          replaceDuplicates(expression, _arg_param_names[i]);
        else
          replaceDuplicates(expression,
                            _arg_param_names[i] + std::to_string(_arg_param_numbers[i]));
      }
      // get variable names from FParser
      _func_F->ParseAndDeduceVariables(expression, _variable_names, false);
      _nargs = _variable_names.size();
      variable_info.resize(_nargs);
      // get our coupled values
      for (unsigned i = 0; i < _arg_param_names.size(); ++i)
      {
        if (_arg_param_numbers[i] < 0)
          setCoupledValues(_arg_param_names[i], _nargs);
        else
          setCoupledValues(_arg_param_names[i], _arg_param_numbers[i]);
      }
      break;

    default:
      mooseError("Unknown variable mapping mode");
  }

  addAllArgNames();

  // get all material properties
  unsigned int nmat_props = mat_prop_expressions.size();
  _mat_prop_descriptors.resize(nmat_props);
  std::vector<std::string>::iterator finder;
  for (unsigned int i = 0; i < nmat_props; ++i)
  {
    // parse the material property parameter entry into a FunctionMaterialPropertyDescriptor
    _mat_prop_descriptors[i] = FunctionMaterialPropertyDescriptor(mat_prop_expressions[i], this);
    finder = std::find(_variable_names.begin(), _variable_names.end(), mat_prop_expressions[i]);
    variable_info[std::distance(_variable_names.begin(), finder)] =
        std::make_tuple<VariableType, unsigned int, unsigned int>(MATERIAL_PROPERTY, 1 * i, 9);
  }

  // create parameter passing buffer
  _func_params.resize(_nargs);
  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

void
ParsedMaterialHelper::setCoupledValues(const std::string & var_to_find, unsigned int component)
{
  std::vector<std::string> comps{"x", "y", "z"};
  std::vector<std::string>::iterator finder;
  std::string to_find;
  if (component < _nargs)
    to_find = var_to_find + std::to_string(component);
  else
    to_find = var_to_find;

  // we will look to see if the variable is being used in the equation
  finder = std::find(_variable_names.begin(), _variable_names.end(), to_find);
  // if it is, we now find the value of the variable
  if (finder != _variable_names.end())
  {
    variable_info[std::distance(_variable_names.begin(), finder)] =
        std::make_tuple<VariableType, unsigned int, unsigned int>(VARIABLE_VALUE, _args.size(), 9);
    if (component < _nargs)
      _args.push_back(&coupledValue(var_to_find, component));
    else
    {
      if (_map_mode == USE_MOOSE_NAMES)
        _args.push_back(&coupledValue(map_to_arg_names[var_to_find]));
      else
        _args.push_back(&coupledValue(var_to_find));
    }
  }
  // do the same for all of the gradient components
  for (unsigned int i = 0; i < 3; ++i)
  {
    finder = std::find(_variable_names.begin(), _variable_names.end(), to_find + comps[i]);
    if (finder != _variable_names.end())
    {
      variable_info[std::distance(_variable_names.begin(), finder)] =
          std::make_tuple<VariableType, unsigned int, unsigned int>(
              VARIABLE_GRADIENT, _grad_args.size(), 1 * i);
      if (component < _nargs)
        _grad_args.push_back(&coupledGradient(var_to_find, component));
      else
      {
        if (_map_mode == USE_MOOSE_NAMES)
          _grad_args.push_back(&coupledGradient(map_to_arg_names[var_to_find]));
        else
          _grad_args.push_back(&coupledGradient(var_to_find));
      }
    }
    // do the same for all of the second derivative components
    for (unsigned int j = 0; j < 3; ++j)
    {
      finder =
          std::find(_variable_names.begin(), _variable_names.end(), to_find + comps[i] + comps[j]);
      if (finder != _variable_names.end())
      {
        variable_info[std::distance(_variable_names.begin(), finder)] =
            std::make_tuple<VariableType, unsigned int, unsigned int>(
                VARIABLE_SECOND, _second_args.size(), 3 * i + j);
        if (component < _nargs)
          _second_args.push_back(&coupledSecond(var_to_find, component));
        else
        {
          if (_map_mode == USE_MOOSE_NAMES)
            _second_args.push_back(&coupledSecond(map_to_arg_names[var_to_find]));
          else
            _second_args.push_back(&coupledSecond(var_to_find));
        }
      }
    }
  }
}

void
ParsedMaterialHelper::replaceDuplicates(std::string & expression, const std::string & to_replace)
{
  std::vector<std::string> comps{"yx", "zx", "zy"};
  for (auto & comp : comps)
  {
    std::size_t variable_location = expression.find(to_replace + comp);
    char temp;
    while (variable_location != std::string::npos)
    {
      temp = expression[variable_location + to_replace.size()];
      expression[variable_location + to_replace.size()] =
          expression[variable_location + to_replace.size() + 1];
      expression[variable_location + to_replace.size() + 1] = temp;
      variable_location = expression.find(to_replace + comp);
    }
  }
}

void
ParsedMaterialHelper::addAllArgNames()
{
  // we need this to check is any of the material properties
  // have derivative components
  std::vector<std::string> comps{"x", "y", "z"};
  std::vector<std::string> temp_args = _arg_names;
  for (auto & arg : temp_args)
    for (auto & comp1 : comps)
    {
      _arg_names.push_back(arg + comp1);
      if (_map_mode == USE_PARAM_NAMES)
        map_to_arg_names[map_to_arg_names[arg] + comp1] = arg + comp1;
      for (auto & comp2 : comps)
      {
        _arg_names.push_back(arg + comp1 + comp2);
        if (_map_mode == USE_PARAM_NAMES)
          map_to_arg_names[map_to_arg_names[arg] + comp1 + comp2] = arg + comp1 + comp2;
      }
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
  // fill the parameter vector
  for (unsigned int i = 0; i < _nargs; ++i)
    _func_params[i] = getValue(i);

  // set function value
  if (_prop_F)
    (*_prop_F)[_qp] = evaluate(_func_F);
}

Real
ParsedMaterialHelper::getValue(unsigned int index)
{
  switch (std::get<0>(variable_info[index]))
  {
    case VARIABLE_VALUE:
      return (*_args[std::get<1>(variable_info[index])])[_qp];
    case VARIABLE_GRADIENT:
      return ((*_grad_args[std::get<1>(variable_info[index])])[_qp])(
          std::get<2>(variable_info[index]));
    case VARIABLE_SECOND:
      return ((*_second_args[std::get<1>(variable_info[index])])[_qp])(
          std::get<2>(variable_info[index]) / 3, std::get<2>(variable_info[index]) % 3);
    case MATERIAL_PROPERTY:
      return _mat_prop_descriptors[std::get<1>(variable_info[index])].value()[_qp];
  }
  return 0;
}
