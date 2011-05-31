#include "TemperatureAux.h"

template<>
InputParameters validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Mark variables as required
  params.addRequiredCoupledVar("p", "");
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // Only required in 3D...
  params.addRequiredCoupledVar("pe", "");
  params.addRequiredCoupledVar("c_v", "");

  return params;
}



TemperatureAux::TemperatureAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _p(coupledValue("p")),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pe(coupledValue("pe")),
   _c_v(coupledValue("c_v"))//,
   //_c_v(getMaterialProperty<Real>("c_v"))
{}


Real
TemperatureAux::computeValue()
{
  Real V2 = 
    _u_vel[_qp]*_u_vel[_qp] + 
    _v_vel[_qp]*_v_vel[_qp] + 
    _w_vel[_qp]*_w_vel[_qp];
  
  // Internal Energy = Total Energy - Kinetic 
  Real e_i = (_pe[_qp] / _p[_qp]) - 0.5*V2;

  // std::cout << "TemperatureAux::computeValue() e_i=" << e_i << std::endl;
  
  // If this is zero, that's bad, you are about to divide by it!
  // std::cout << "TemperatureAux::computeValue() _c_v[_qp]=" << _c_v[_qp] << std::endl;

  // T = e_i / c_v
  return e_i / _c_v[_qp];
}
