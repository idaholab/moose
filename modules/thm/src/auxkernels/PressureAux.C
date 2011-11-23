#include "PressureAux.h"

template<>
InputParameters validParams<PressureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Coupled variables
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("T", "");
  
  // Parameters
  params.addRequiredParam<Real>("rho_0", "Reference density");
  params.addRequiredParam<Real>("p_0", "Reference pressure");
  params.addRequiredParam<Real>("drho_dp", "Change in density wrt pressure, at reference state");
  params.addRequiredParam<Real>("T_0", "Reference temperature");
  params.addRequiredParam<Real>("drho_dT", "Change in density wrt temperature, at reference state");
  
  return params;
}

PressureAux::PressureAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _rho(coupledValue("rho")),
   _T(coupledValue("T")),
   _rho_0(getParam<Real>("rho_0")),
   _p_0(getParam<Real>("p_0")),
   _drho_dp(getParam<Real>("drho_dp")),
   _T_0(getParam<Real>("T_0")),
   _drho_dT(getParam<Real>("drho_dT"))   
{}

Real
PressureAux::computeValue()
{
  // Inversion of the linear approimxation: rho = rho_0 + (drho/dp)|_0 * (p - p_0) + (drho/dT)|_0 * (T - T_0)
  // to obtain pressure
  return _p_0 + ( (_rho[_qp] - _rho_0) - _drho_dT * (_T[_qp] - _T_0 ) ) / _drho_dp;
}
