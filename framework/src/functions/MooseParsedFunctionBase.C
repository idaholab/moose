//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseParsedFunctionBase.h"

// MOOSE includes
#include "InputParameters.h"
#include "MooseError.h"
#include "MooseParsedFunctionWrapper.h"

InputParameters
MooseParsedFunctionBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addDeprecatedParam<std::vector<std::string>>(
      "vars",
      "Variables (excluding t,x,y,z) that are bound to the values provided by the corresponding "
      "items in the vals vector.",
      "Use 'symbol_names' instead.");
  params.addDeprecatedParam<std::vector<std::string>>(
      "vals",
      "Constant numeric values, postprocessor names, "
      "function names, and scalar variables for vars.",
      "Use 'symbol_values' instead.");
  params.addParam<std::vector<std::string>>(
      "symbol_names",
      "Symbols (excluding t,x,y,z) that are bound to the values provided by the corresponding "
      "items in the vals vector.");
  params.addParam<std::vector<std::string>>("symbol_values",
                                            "Constant numeric values, postprocessor names, "
                                            "function names, and scalar variables corresponding to"
                                            " the symbols in symbol_names.");
  return params;
}

MooseParsedFunctionBase::MooseParsedFunctionBase(const InputParameters & parameters)
  : _pfb_feproblem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vars(parameters.isParamValid("vars")
              ? parameters.get<std::vector<std::string>>("vars")
              : parameters.get<std::vector<std::string>>("symbol_names")),
    _vals(parameters.isParamValid("vals")
              ? parameters.get<std::vector<std::string>>("vals")
              : parameters.get<std::vector<std::string>>("symbol_values"))
{
  if (_vars.size() != _vals.size())
    mooseError("Number of symbol_names must match the number of symbol_values!");

  // Loop through the variables assigned by the user and give an error if x,y,z,t are used
  for (const auto & var : _vars)
    if (var.find_first_of("xyzt") != std::string::npos && var.size() == 1)
      mooseError("The variables \"x, y, z, and t\" in the ParsedFunction are pre-declared for use "
                 "and must not be declared in \"vars\"");
}

MooseParsedFunctionBase::~MooseParsedFunctionBase() {}

const std::string
MooseParsedFunctionBase::verifyFunction(const std::string & function_str)
{
  // Throws an error if quotes are found
  if (function_str.find("\"") != std::string::npos)
    mooseError("The expression in ParsedFunction contains quotes which cannot be properly parsed");

  // Return the input equation (no error)
  return function_str;
}
