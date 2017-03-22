/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ParsedMaterialBase.h"

template <>
InputParameters
validParams<ParsedMaterialBase>()
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

  // Function expression
  params.addRequiredParam<std::string>("function",
                                       "FParser function expression for the phase free energy");

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
