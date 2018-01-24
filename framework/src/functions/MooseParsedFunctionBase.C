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

template <>
InputParameters
validParams<MooseParsedFunctionBase>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::string>>(
      "vars", "The constant variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<std::string>>(
      "vals", "Constant numeric values or postprocessor names for vars.");
  return params;
}

MooseParsedFunctionBase::MooseParsedFunctionBase(const InputParameters & parameters)
  : _pfb_feproblem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vars(parameters.get<std::vector<std::string>>("vars")),
    _vals(parameters.get<std::vector<std::string>>("vals"))
{
  if (_vars.size() != _vals.size())
    mooseError("Number of vars must match the number of vals for a MooseParsedFunction!");

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
    mooseError("The value in ParsedFunction contains quotes(\") which cannot be properly parsed");

  // Return the input equation (no error)
  return function_str;
}
