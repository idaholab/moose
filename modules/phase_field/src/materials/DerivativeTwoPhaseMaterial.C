#include "DerivativeTwoPhaseMaterial.h"

template<>
InputParameters validParams<DerivativeTwoPhaseMaterial>()
{
  InputParameters params = validParams<DerivativeTwoPhaseMaterialHelper>();
  params.addClassDescription("Two phase material that combines two single phase materials using a switching function.");

  // Two base materials
  params.addRequiredParam<std::string>("fa_name", "Phase A material (at phi=0)");
  params.addRequiredParam<std::string>("fb_name", "Phase A material (at phi=1)");

  // Order parameter which determines the phase
  parama.addCoupledVariable("phi", "Order parameter")

  // Variables with applied tolerances and their tolerance values
  params.addParam<Real>("W", 0.0, "Energy barrier for the phase transformation from A to B");
  params.addParam<std::vector<Real> >( "tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  return params;
}

DerivativeTwoPhaseMaterial::DerivativeTwoPhaseMaterial(const std::string & name,
                                                       InputParameters parameters) :
    DerivativeMaterial(name, parameters),
    _fa_name(getParam<std::string>("fa_name")),
    _fb_name(getParam<std::string>("fb_name")),
    _phi(goupledValue("phi")),
    _W(getParam<Real>("W"))
{
  // check number of coupled variables
  if (_arg_names.size() == 0)
    mooseError("Need at least one couple variable for DerivativeTwoPhaseMaterial.");

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
DerivativeTwoPhaseMaterial::expectedNumArgs()
{
  // this alwats returns the number of argumens that was passed in
  // i.e. any number of args is accepted.
  return _nargs;
}
