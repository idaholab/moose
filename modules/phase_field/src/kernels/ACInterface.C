/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterface.h"

template<>
InputParameters validParams<ACInterface>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  return params;
}

ACInterface::ACInterface(const std::string & name, InputParameters parameters) :
    KernelGrad(name, parameters),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _L(getMaterialProperty<Real>("mob_name"))
{
}

RealGradient
ACInterface::precomputeQpResidual()
{
  return _kappa[_qp] * _L[_qp] * _grad_u[_qp];
}

RealGradient
ACInterface::precomputeQpJacobian()
{
  return _kappa[_qp] * _L[_qp] * _grad_phi[_j][_qp];
}
