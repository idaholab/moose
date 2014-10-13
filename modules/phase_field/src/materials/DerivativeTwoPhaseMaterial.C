#include "DerivativeTwoPhaseMaterial.h"

template<>
InputParameters validParams<DerivativeTwoPhaseMaterial>()
{
  InputParameters params = validParams<DerivativeTwoPhaseMaterial>();
  params.addClassDescription("Two phase material that combines two single phase materials using a switching function.");

  // Two base materials
  params.addRequiredParam<std::string>("fa_name", "Phase A material (at phi=0)");
  params.addRequiredParam<std::string>("fb_name", "Phase A material (at phi=1)");

  // Order parameter which determines the phase
  params.addCoupledVar("phi", "Order parameter");

  // Variables with applied tolerances and their tolerance values
  params.addParam<Real>("W", 0.0, "Energy barrier for the phase transformation from A to B");
  params.addParam<std::vector<Real> >("tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  return params;
}

InputParameters
DerivativeTwoPhaseMaterial::AddArg(InputParameters params, VariableName var_name)
{
  params.set<std::vector<VariableName> >("args").push_back(var_name);
  return params;
}

DerivativeTwoPhaseMaterial::DerivativeTwoPhaseMaterial(const std::string & name,
                                                       InputParameters parameters) :
    DerivativeBaseMaterial(name, AddArg(parameters, getVar("phi", 0)->name())),
    _fa_name(getParam<std::string>("fa_name")),
    _fb_name(getParam<std::string>("fb_name")),
    _phi(coupledValue("phi")),
    _W(getParam<Real>("W"))
{
}

/// Fm(cmg,cmv,T) takes three arguments
unsigned int
DerivativeTwoPhaseMaterial::expectedNumArgs()
{
  // this alwats returns the number of argumens that was passed in
  // i.e. any number of args is accepted.
  return _nargs;
}
