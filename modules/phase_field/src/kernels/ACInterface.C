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
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

ACInterface::ACInterface(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(name, parameters),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
}

RealGradient
ACInterface::precomputeQpResidual()
{
  // Set interfacial part of residual
  return _kappa[_qp] * _L[_qp] * _grad_u[_qp];
}

RealGradient
ACInterface::precomputeQpJacobian()
{
  // Set Jacobian using product rule
  return _kappa[_qp] * (_L[_qp] * _grad_phi[_j][_qp] + _dLdop[_qp] * _phi[_j][_qp] * _grad_u[_qp]);
}

Real
ACInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
  return _kappa[_qp] * (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}
