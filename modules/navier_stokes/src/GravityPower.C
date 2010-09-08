#include "GravityPower.h"
 
template<>
InputParameters validParams<GravityPower>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("acceleration") = -9.80665;
  return params;
}

GravityPower::GravityPower(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _pv_var(coupled("pv")),
    _pv(coupledValue("pv")),
    _acceleration(getParam<Real>("acceleration"))
  {}

Real
GravityPower::computeQpResidual()
{
  return -_pv[_qp]*_acceleration*_test[_i][_qp];
}

Real
GravityPower::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _pv_var)
  {
    return -_phi[_j][_qp]*_acceleration*_test[_i][_qp];
  }

  return 0;
}
