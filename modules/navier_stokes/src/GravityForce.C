#include "GravityForce.h"
 
template<>
InputParameters validParams<GravityForce>()
{
  InputParameters params = validParams<Kernel>();

  // This term is coupled to the density
  params.addRequiredCoupledVar("rho", "");

  // Defaults to earth gravity
  params.addRequiredParam<Real>("acceleration", "The body force vector component.");

  return params;
}

GravityForce::GravityForce(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
    _rho_var(coupled("rho")),
    _rho(coupledValue("rho")),
    _acceleration(getParam<Real>("acceleration"))
  {}

Real
GravityForce::computeQpResidual()
{
  // -rho * g * phi
  return -_rho[_qp]*_acceleration*_test[_i][_qp];
}

Real
GravityForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _rho_var)
  {
    return -_phi[_j][_qp]*_acceleration*_test[_i][_qp];
  }

  return 0;
}
