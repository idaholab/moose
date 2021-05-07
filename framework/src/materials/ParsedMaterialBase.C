//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedMaterialBase.h"

InputParameters
ParsedMaterialBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addCoupledVar("args", "Arguments of F() - use vector coupling");

  // Constants and their values
  params.addParam<std::vector<std::string>>(
      "constant_names",
      std::vector<std::string>(),
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      std::vector<std::string>(),
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  // Variables with applied tolerances and their tolerance values
  params.addParam<std::vector<std::string>>("tol_names",
                                            std::vector<std::string>(),
                                            "Vector of variable names to be protected from "
                                            "being 0 or 1 within a tolerance (needed for log(c) "
                                            "and log(1-c) terms)");
  params.addParam<std::vector<Real>>("tol_values",
                                     std::vector<Real>(),
                                     "Vector of tolerance values for the variables in tol_names");

  // Material properties
  params.addParam<std::vector<std::string>>(
      "material_property_names",
      std::vector<std::string>(),
      "Vector of material properties used in the parsed function");

  // Postprocessors
  params.addParam<std::vector<PostprocessorName>>(
      "postprocessor_names",
      std::vector<PostprocessorName>(),
      "Vector of postprocessor names used in the parsed function");

  // Function expression
  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "FParser function expression for the parsed material");

  return params;
}

ParsedMaterialBase::ParsedMaterialBase(const InputParameters & parameters)
{
  // get function expression
  _function = parameters.get<std::string>("function");

  // get constant vectors
  _constant_names = parameters.get<std::vector<std::string>>("constant_names");
  _constant_expressions = parameters.get<std::vector<std::string>>("constant_expressions");

  // get tolerance vectors
  _tol_names = parameters.get<std::vector<std::string>>("tol_names");
  _tol_values = parameters.get<std::vector<Real>>("tol_values");
}
