#include "NSTemperatureAux.h"

template<>
InputParameters validParams<NSTemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Mark variables as required
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // Only required in 3D...
  params.addRequiredCoupledVar("rhoe", "");

  // Parameters with default values
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  return params;
}



NSTemperatureAux::NSTemperatureAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _rho(coupledValue("rho")),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _rhoe(coupledValue("rhoe")),
   _cv(getParam<Real>("cv"))
{}


Real
NSTemperatureAux::computeValue()
{
  Real V2 = 
    _u_vel[_qp]*_u_vel[_qp] + 
    _v_vel[_qp]*_v_vel[_qp] + 
    _w_vel[_qp]*_w_vel[_qp];
  
  // Internal Energy = Total Energy - Kinetic 
  Real e_i = (_rhoe[_qp] / _rho[_qp]) - 0.5*V2;

  // T = e_i / c_v
  return e_i / _cv;
}
