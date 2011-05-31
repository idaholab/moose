#include "NSPressureAux.h"

template<>
InputParameters validParams<NSPressureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Mark variables as required
  params.addRequiredCoupledVar("p", "");
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // Only required in 3D...
  params.addRequiredCoupledVar("pe", "");
  
  // Parameters with default values
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  return params;
}

NSPressureAux::NSPressureAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _p(coupledValue("p")),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pe(coupledValue("pe")),
   _gamma(getParam<Real>("gamma")) // can't use Material properties in Nodal Aux...
{}

Real
NSPressureAux::computeValue()
{
  // Velocity vector, squared
  Real V2 = _u_vel[_qp]*_u_vel[_qp] + _v_vel[_qp]*_v_vel[_qp] + _w_vel[_qp]*_w_vel[_qp];

  // P = (gam-1) * ( rho*e_t - 1/2 * rho * V^2)
  return (_gamma - 1)*(_pe[_qp] - 0.5 * _p[_qp] * V2);
}
