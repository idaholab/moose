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

defineLegacyParams(ParsedMaterialHelper);

InputParameters
ParsedMaterialHelper::validParams()
{

  InputParameters params = FunctionMaterialBase::validParams();
  params += FunctionParserUtils::validParams();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

ParsedMaterialHelper::ParsedMaterialHelper(const InputParameters & parameters,
                                           VariableNameMappingMode map_mode)
  : FunctionMaterialBase(parameters),
    FunctionParserUtils(parameters),
    _variable_names(0),
    _mat_prop_descriptors(0),
    _tol(0),
    _map_mode(map_mode),
    variable_info(0)
{
  switch (map_mode)
  {
    case USE_MOOSE_NAMES:
      _equation_args = _arg_names;
      break;
    case USE_PARAM_NAMES:
      _equation_args = _arg_param_names;
      break;
  }
  _args.empty();
  for (unsigned int i = 0; i < _equation_args.size(); ++i)
  {
    if (!_arg_param_unique[i] && _map_mode == USE_PARAM_NAMES)
    {
      _equation_args[i] += std::to_string(_arg_param_numbers[i]);
      _arg_names[i] += std::to_string(_arg_param_numbers[i]);
    }
  }
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
  _func_F = std::make_shared<ADFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

  // add further constants coming from default value coupling
  if (_map_mode == USE_PARAM_NAMES)
    for (const auto & acd : _arg_constant_defaults)
      if (!_func_F->AddConstant(acd, _pars.defaultCoupledValue(acd)))
        mooseError("Invalid constant name in parsed function object");

  // replace duplicate derivative terms (i.e. D[c,xy] == D[c,yx])
  for (unsigned int i = 0; i < _nargs; ++i)
    replaceDuplicates(expression, _equation_args[i]);

  // get variable names from FParser
  _func_F->ParseAndDeduceVariables(expression, _variable_names, false);
  _nargs = _variable_names.size();

  variable_info.resize(_nargs);

  for (auto & var : _variable_names)
    getVariableValue(var, mat_prop_expressions);

  // get all possible variable names for derivatives of material properties
  addAllArgNames();

  // create parameter passing buffer
  _func_params.resize(_nargs);
  // perform next steps (either optimize or take derivatives and then optimize)

  functionsPostParse();
}

void
ParsedMaterialHelper::replaceDuplicates(std::string & expression, const std::string & to_replace)
{
  std::vector<std::string> comps{"_yx", "_zx", "_zy"};
  for (auto & comp : comps)
  {
    std::size_t variable_location = expression.find(to_replace + comp);
    char temp;
    while (variable_location != std::string::npos)
    {
      temp = expression[variable_location + to_replace.size() + 1];
      expression[variable_location + to_replace.size() + 1] =
          expression[variable_location + to_replace.size() + 2];
      expression[variable_location + to_replace.size() + 2] = temp;
      variable_location = expression.find(to_replace + comp);
    }
  }
}

void
ParsedMaterialHelper::getVariableValue(const std::string & var,
                                       const std::vector<std::string> & mat_prop_expressions)
{

  std::map<char, unsigned int> char_to_int = {{'x', 0}, {'y', 1}, {'z', 2}};
  std::string::const_iterator find_comps;
  std::vector<std::string>::iterator find_var;
  std::vector<std::string>::const_iterator find_mat_prop;

  VariableInfo this_var_info;
  std::string var_name = var;
  std::string comps;

  // Find where the variable ends and the components begin
  for (find_comps = var.begin(); find_comps != var.end(); ++find_comps)
    if (*find_comps == '_' &&
        (*(find_comps + 1) == 'x' || *(find_comps + 1) == 'y' || *(find_comps + 1) == 'z'))
    {
      var_name = std::string(var.begin(), find_comps);
      comps = std::string(find_comps + 1, var.end());
    }
  // If the variable is a coupled variable
  for (find_var = _equation_args.begin(); find_var != _equation_args.end(); ++find_var)
    if (var_name == *find_var)
    {
      unsigned int arg_index = find_var - _equation_args.begin();
      if (!comps.empty())
      {
        char last_comp = comps.back();
        comps.pop_back();
        if (!comps.empty())
        {
          this_var_info.comp_one = char_to_int[comps.back()];
          this_var_info.comp_two = char_to_int[last_comp];
          this_var_info.var_type = VARIABLE_SECOND;
          this_var_info.arg_location = _second_args.size();
          _second_args.push_back(
              &coupledSecond(_arg_param_names[arg_index], _arg_param_numbers[arg_index]));
          variable_info.push_back(this_var_info);
          return;
        }
        this_var_info.comp_one = char_to_int[last_comp];
        this_var_info.var_type = VARIABLE_GRADIENT;
        this_var_info.arg_location = _grad_args.size();
        _grad_args.push_back(
            &coupledGradient(_arg_param_names[arg_index], _arg_param_numbers[arg_index]));
        variable_info.push_back(this_var_info);
        return;
      }
      this_var_info.var_type = VARIABLE_VALUE;
      this_var_info.arg_location = _args.size();
      _args.push_back(&coupledValue(_arg_param_names[arg_index], _arg_param_numbers[arg_index]));
      variable_info.push_back(this_var_info);
      return;
    }
  // If the variable is a material property
  for (find_mat_prop = mat_prop_expressions.begin(); find_mat_prop != mat_prop_expressions.end();
       ++find_mat_prop)
  {
    std::string clean_mat;
    cleanMat(*find_mat_prop, clean_mat);
    if (var_name == clean_mat)
    {
      this_var_info.var_type = MATERIAL_PROPERTY;
      this_var_info.arg_location = _mat_prop_descriptors.size();
      _mat_prop_descriptors.push_back(FunctionMaterialPropertyDescriptor(*find_mat_prop, this));
      variable_info.push_back(this_var_info);
      return;
    }
  }
  // The variable isn't in the coupled variables or material properties
  mooseError("Unknown Variable Name!");
}

void
ParsedMaterialHelper::cleanMat(const std::string & dirty_mat, std::string & clean_mat)
{
  std::string::const_iterator find_dirt;
  for (find_dirt = dirty_mat.begin(); find_dirt != dirty_mat.end(); ++find_dirt)
    if (*find_dirt == '(' || *find_dirt == ':')
    {
      clean_mat = std::string(dirty_mat.begin(), find_dirt);
      return;
    }
  clean_mat = dirty_mat;
  return;
}

void
ParsedMaterialHelper::addAllArgNames()
{
  // we need this to check for all possible components of a coupled variable
  std::vector<std::string> comps{"x", "y", "z"};
  unsigned int numVars = _arg_names.size();
  for (unsigned int i = 0; i < numVars; ++i)
    for (unsigned int j = 0; j < 3; ++j)
    {
      _arg_names.push_back(_arg_names[i] + "_" + comps[j]);
      _equation_args.push_back(_equation_args[i] + "_" + comps[j]);
      for (unsigned int k = 0; k < 3; ++k)
      {
        _arg_names.push_back(_arg_names[i] + "_" + comps[j] + comps[k]);
        _equation_args.push_back(_equation_args[i] + "_" + comps[j] + comps[k]);
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
  switch (variable_info[index].var_type)
  {
    case VARIABLE_VALUE:
      return (*_args[variable_info[index].arg_location])[_qp];
    case VARIABLE_GRADIENT:
      return ((*_grad_args[variable_info[index].arg_location])[_qp])(variable_info[index].comp_one);
    case VARIABLE_SECOND:
      return ((*_second_args[variable_info[index].arg_location])[_qp])(
          variable_info[index].comp_one, variable_info[index].comp_two);
    case MATERIAL_PROPERTY:
      return _mat_prop_descriptors[variable_info[index].arg_location].value()[_qp];
    case NO_VAL:
      return 0;
  }
  return 0;
}
