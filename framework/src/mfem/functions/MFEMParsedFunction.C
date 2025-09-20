//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMParsedFunction.h"
#include "MFEMScalarParsedCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMParsedFunction);

InputParameters
MFEMParsedFunction::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.registerBase("Function");
  params.addClassDescription("Parses function expression of position, time and problem "
                             "coefficients (including problem variables).");
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Parsed function expression to compute");
  params.addParam<std::vector<MFEMScalarCoefficientName>>(
      "coefficients", {}, "The names of the function coefficients");
  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression");
  params.addParam<std::vector<std::string>>(
      "constant_names",
      {},
      "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  return params;
}

MFEMParsedFunction::MFEMParsedFunction(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    FunctionParserUtils(parameters),
    _coef_names(getParam<std::vector<MFEMScalarCoefficientName>>("coefficients")),
    _use_xyzt(getParam<bool>("use_xyzt")),
    _xyzt({"x", "y", "z", "t"})
{
  // coefficients (including any variables) the function depends on
  std::string symbols = MooseUtils::stringJoin({_coef_names.begin(), _coef_names.end()}, ",");

  // positions and time
  if (_use_xyzt)
    symbols += (symbols.empty() ? "" : ",") + MooseUtils::stringJoin(_xyzt, ",");

  // create parsed function
  _func_F = std::make_shared<SymFunction>();
  parsedFunctionSetup(_func_F,
                      getParam<std::string>("expression"),
                      symbols,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"),
                      comm());

  // create MFEMScalarParsedCoefficient
  getMFEMProblem().getCoefficients().declareScalar<MFEMScalarParsedCoefficient>(
      name(), getMFEMProblem().getCoefficients(), _coef_names, _use_xyzt, _func_F);
}

#endif
