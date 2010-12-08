#include "CHInterface.h"

template<>
InputParameters validParams<CHInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("kappa_name","The kappa used with the kernel");
  params.addRequiredParam<std::string>("mob_name","The mobility used with the kernel");
  params.addRequiredParam<std::string>("grad_mob_name","The gradient of the mobility used with the kernel");
  
  return params;
}

CHInterface::CHInterface(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _kappa_name(getParam<std::string>("kappa_name")),
   _mob_name(getParam<std::string>("mob_name")),
   _grad_mob_name(getParam<std::string>("grad_mob_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name)),
   _M(getMaterialProperty<Real>(_mob_name)),
   _grad_M(getMaterialProperty<RealGradient>(_grad_mob_name))
{
}

Real
CHInterface::computeQpResidual()
{
  //Actual value to return
  Real value = 0.0;
  
  value += _kappa[_qp]*_second_u[_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]);
  
  return value;
}

Real
CHInterface::computeQpJacobian()
{
  //Actual value to return
  Real value = 0.0;

  value += _kappa[_qp]*_second_phi[_j][_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]);
  
  return value;
}
  
