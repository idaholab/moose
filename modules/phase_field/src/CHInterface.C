#include "CHInterface.h"

template<>
InputParameters validParams<CHInterface>()
{
  InputParameters params = validParams<Kernel>();
  
  return params;
}

CHInterface::CHInterface(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _kappa_c(getMaterialProperty<Real>("kappa_c")),
   _M(getMaterialProperty<Real>("M")),
   _grad_M(getMaterialProperty<RealGradient>("grad_M"))
{
}

Real
CHInterface::computeQpResidual()
{
  //Actual value to return
  Real value = 0.0;
  
  value += 2.0*_kappa_c[_qp]*(_second_u[_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]));
  
  return value;
}

Real
CHInterface::computeQpJacobian()
{
  //Actual value to return
  Real value = 0.0;

  value += 2.*_kappa_c[_qp]*_second_phi[_j][_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]);
  
  return value;
}
  
