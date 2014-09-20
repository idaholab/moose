#include "DerivativeParsedMaterial.h"

template<>
InputParameters validParams<DerivativeParsedMaterial>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");

  // Constants and their values
  params.addParam<std::vector<std::string> >("constant_names", std::vector<std::string>(), "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string> >( "constant_expressions", std::vector<std::string>(), "Vector of values for the constants in constant_names (can be an FParser expression)");

  // Variables with applied tolerances and their tolerance values
  params.addParam<std::vector<std::string> >("tol_names", std::vector<std::string>(), "Vector of variable names to be protected from being 0 or 1 within a tolerance (needed for log(c) and log(1-c) terms)");
  params.addParam<std::vector<Real> >( "tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  // Material properties
  params.addParam<std::vector<std::string> >(
    "material_property_names", std::vector<std::string>(),
    "Vector of material properties used in the parsed function (these should have a zero gradient!)");

  // Function expression
  params.addRequiredParam<std::string>("function", "FParser function expression for the phase free energy");

  return params;
}

DerivativeParsedMaterial::DerivativeParsedMaterial(const std::string & name,
                                                   InputParameters parameters) :
    DerivativeParsedMaterialHelper(name, parameters)
{
  // check number of coupled variables
  if (_arg_names.size() == 0)
    mooseError("Need at least one couple variable for DerivativeParsedMaterial.");

  // get constant vectors
  std::vector<std::string> constant_names = getParam<std::vector<std::string> >("constant_names");
  std::vector<std::string> constant_expressions = getParam<std::vector<std::string> >("constant_expressions");

  // get tolerance vectors
  std::vector<std::string> tol_names = getParam<std::vector<std::string> >("tol_names");
  std::vector<Real> tol_values = getParam<std::vector<Real> >("tol_values");

  // get material property names
  std::vector<std::string> mat_prop_names = getParam<std::vector<std::string> >("material_property_names");

  // Build function and derivatives
  functionParse(getParam<std::string>("function"),
                constant_names, constant_expressions,
                mat_prop_names,
                tol_names, tol_values);
}

/// Fm(cmg,cmv,T) takes three arguments
unsigned int
DerivativeParsedMaterial::expectedNumArgs()
{
  // this alwats returns the number of argumens that was passed in
  // i.e. any number of args is accepted.
  return _nargs;
}
