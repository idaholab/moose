#include "MFEMParsedCoefficientBase.h"

InputParameters
MFEMParsedCoefficientBase::validParams()
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

  // Material properties
  params.addParam<std::vector<std::string>>(
      "mfem_gridfunction_names",
      std::vector<std::string>(),
      "Vector of MFEM gridfunctions names used in the parsed function");

  // Postprocessors
  params.addParam<std::vector<std::string>>(
      "mfem_coefficient_names",
      std::vector<std::string>(),
      "Vector of MFEM coefficient names used in the parsed function");

  // Function expression
  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "FParser function expression for the parsed material");

  return params;
}

MFEMParsedCoefficientBase::MFEMParsedCoefficientBase(const InputParameters & parameters)
{
  // get function expression
  _function = parameters.get<std::string>("function");

  // get constant vectors
  _constant_names = parameters.get<std::vector<std::string>>("constant_names");
  _constant_expressions = parameters.get<std::vector<std::string>>("constant_expressions");

  // get mfem object names
  _mfem_coefficient_names = parameters.get<std::vector<std::string>>("mfem_coefficient_names");
  _mfem_gridfunction_names = parameters.get<std::vector<std::string>>("mfem_gridfunction_names");
}
