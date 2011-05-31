#include "GravityPower.h"
 
template<>
InputParameters validParams<GravityPower>()
{
  InputParameters params = validParams<Kernel>();

  // This term is coupled to the momentum variable for whichever equation this is
  params.addRequiredCoupledVar("momentum", "");
  
  // Defaults to earth gravity
  params.addParam<Real>("acceleration", -9.80665, "The acceleration due to gravity.");

  return params;
}

GravityPower::GravityPower(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
    _momentum_var(coupled("momentum")),
    _momentum(coupledValue("momentum")),
    _acceleration(getParam<Real>("acceleration"))
  {}

Real
GravityPower::computeQpResidual()
{
  // -(rho * U * g) * phi
  return -_momentum[_qp]*_acceleration*_test[_i][_qp];
}

Real
GravityPower::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _momentum_var)
  {
    return -_phi[_j][_qp]*_acceleration*_test[_i][_qp];
  }

  return 0;
}
