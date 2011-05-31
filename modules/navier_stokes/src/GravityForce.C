#include "GravityForce.h"
 
template<>
InputParameters validParams<GravityForce>()
{
  InputParameters params = validParams<Kernel>();

  // This term is coupled to the density
  params.addRequiredCoupledVar("p", "");

  // Defaults to earth gravity
  params.addParam<Real>("acceleration", -9.80665, "The acceleration due to gravity.");

  return params;
}

GravityForce::GravityForce(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
    _p_var(coupled("p")),
    _p(coupledValue("p")),
    _acceleration(getParam<Real>("acceleration"))
  {}

Real
GravityForce::computeQpResidual()
{
  // -rho * g * phi
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
