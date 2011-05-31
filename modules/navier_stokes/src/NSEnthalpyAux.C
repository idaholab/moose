#include "NSEnthalpyAux.h"

template<>
InputParameters validParams<NSEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Mark variables as required
  params.addRequiredCoupledVar("p", "");
  params.addRequiredCoupledVar("pe", "");
  params.addRequiredCoupledVar("pressure", "");
  
  // Parameters with default values
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  return params;
}

NSEnthalpyAux::NSEnthalpyAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _p(coupledValue("p")),
   _pe(coupledValue("pe")),
   _pressure(coupledValue("pressure")),
   _gamma(getParam<Real>("gamma")) 
{}

Real
NSEnthalpyAux::computeValue()
{
  // H = (rho*E + P) / rho
  return (_pe[_qp] + _pressure[_qp]) / _p[_qp];
}
