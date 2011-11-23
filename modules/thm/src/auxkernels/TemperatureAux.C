#include "TemperatureAux.h"

template<>
InputParameters validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Coupled variables
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("rhoE", "");
  params.addRequiredCoupledVar("u", "");
  
  // Parameters
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume, J/kg-K");
  
  return params;
}

TemperatureAux::TemperatureAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _rho(coupledValue("rho")),
   _rhoE(coupledValue("rhoE")),
   _vel(coupledValue("u")),
   _cv(getParam<Real>("cv"))   
{}

Real TemperatureAux::computeValue()
{
  return (_rhoE[_qp] / _rho[_qp] - 0.5 * _vel[_qp] * _vel[_qp]) / _cv;
}
