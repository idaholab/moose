#include "GravityForce.h"
 
template<>
InputParameters validParams<GravityForce>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("acceleration") = -9.80665;
  return params;
}

GravityForce::GravityForce(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _p_var(coupled("p")),
    _p(coupledValue("p")),
    _acceleration(getParam<Real>("acceleration"))
  {}

Real
GravityForce::computeQpResidual()
{
  return -_p[_qp]*_acceleration*_test[_i][_qp];
}

Real
GravityForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _p_var)
  {
    return -_phi[_j][_qp]*_acceleration*_test[_i][_qp];
  }

  return 0;
}
