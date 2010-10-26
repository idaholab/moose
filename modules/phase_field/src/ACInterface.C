#include "ACInterface.h"

template<>
InputParameters validParams<ACInterface>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addParam<std::string>("mob_name","L","The mobility used with the kernel");
  params.addParam<std::string>("kappa_name","kappa_op","The kappa used with the kernel");
  
  return params;
}

ACInterface::ACInterface(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :KernelGrad(name, moose_system, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _kappa_name(getParam<std::string>("kappa_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name)),
   _L(getMaterialProperty<Real>(_mob_name))
{ 
}

RealGradient
ACInterface::precomputeQpResidual()
{
  return  _kappa[_qp]*_L[_qp]*( _grad_u[_qp] );
}

RealGradient
ACInterface::precomputeQpJacobian()
{
  return _kappa[_qp]*_L[_qp]*( _grad_phi[_j][_qp] );
}
