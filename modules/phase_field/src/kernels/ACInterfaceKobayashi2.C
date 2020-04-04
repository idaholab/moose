//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACInterfaceKobayashi2.h"

registerMooseObject("PhaseFieldApp", ACInterfaceKobayashi2);

InputParameters
ACInterfaceKobayashi2::validParams()
{
  InputParameters params = JvarMapKernelInterface<KernelGrad>::validParams();
  params.addClassDescription("Anisotropic Gradient energy Allen-Cahn Kernel Part 2");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addParam<MaterialPropertyName>(
      "depsdgrad_op_name",
      "depsdgrad_op",
      "The derivative of the anisotropic interface parameter eps with respect to grad_op");
  return params;
}

ACInterfaceKobayashi2::ACInterfaceKobayashi2(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name")),
    _depsdgrad_op(getMaterialProperty<RealGradient>("depsdgrad_op_name")),
    _dLdarg(_n_args),
    _depsdarg(_n_args)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
  {
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", i);
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", i);
  }
}

RealGradient
ACInterfaceKobayashi2::precomputeQpResidual()
{
  // Set interfacial part of residual
  return _eps[_qp] * _eps[_qp] * _L[_qp] * _grad_u[_qp];
}

RealGradient
ACInterfaceKobayashi2::precomputeQpJacobian()
{
  // Calculate depsdop_i
  Real depsdop_i = _depsdgrad_op[_qp] * _grad_phi[_j][_qp];

  // Set Jacobian using product rule
  return _L[_qp] *
         (_eps[_qp] * _eps[_qp] * _grad_phi[_j][_qp] + 2.0 * _eps[_qp] * depsdop_i * _grad_u[_qp]);
}

Real
ACInterfaceKobayashi2::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // Set off-diagonal jaocbian terms from mobility and epsilon dependence
  Real dsum = _L[_qp] * 2.0 * _eps[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] *
              _grad_test[_i][_qp];
  dsum += _eps[_qp] * _eps[_qp] * (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] *
          _grad_test[_i][_qp];

  return dsum;
}
