//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACInterfaceKobayashi1.h"

registerMooseObject("PhaseFieldApp", ACInterfaceKobayashi1);

InputParameters
ACInterfaceKobayashi1::validParams()
{
  InputParameters params = JvarMapKernelInterface<KernelGrad>::validParams();
  params.addClassDescription("Anisotropic gradient energy Allen-Cahn Kernel Part 1");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic interface parameter");
  params.addParam<MaterialPropertyName>(
      "deps_name",
      "deps",
      "The derivative of the anisotropic interface parameter with respect to angle");
  params.addParam<MaterialPropertyName>(
      "depsdgrad_op_name",
      "depsdgrad_op",
      "The derivative of the anisotropic interface parameter eps with respect to grad_op");
  params.addParam<MaterialPropertyName>(
      "ddepsdgrad_op_name", "ddepsdgrad_op", "The derivative of deps with respect to grad_op");
  return params;
}

ACInterfaceKobayashi1::ACInterfaceKobayashi1(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name")),
    _deps(getMaterialProperty<Real>("deps_name")),
    _depsdgrad_op(getMaterialProperty<RealGradient>("depsdgrad_op_name")),
    _ddepsdgrad_op(getMaterialProperty<RealGradient>("ddepsdgrad_op_name"))
{
  // reserve space for derivatives
  _dLdarg.resize(_n_args);
  _depsdarg.resize(_n_args);
  _ddepsdarg.resize(_n_args);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
  {
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", i);
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", i);
    _ddepsdarg[i] = &getMaterialPropertyDerivative<Real>("deps_name", i);
  }
}

RealGradient
ACInterfaceKobayashi1::precomputeQpResidual()
{
  // Set modified gradient vector
  const RealGradient v(-_grad_u[_qp](1), _grad_u[_qp](0), 0);

  // Define anisotropic interface residual
  return _eps[_qp] * _deps[_qp] * _L[_qp] * v;
}

RealGradient
ACInterfaceKobayashi1::precomputeQpJacobian()
{
  // Set modified gradient vector
  const RealGradient v(-_grad_u[_qp](1), _grad_u[_qp](0), 0);

  // dvdgrad_op*_grad_phi
  const RealGradient dv(-_grad_phi[_j][_qp](1), _grad_phi[_j][_qp](0), 0);

  // Derivative of epsilon wrt nodal op values
  Real depsdop_i = _depsdgrad_op[_qp] * _grad_phi[_j][_qp];
  Real ddepsdop_i = _ddepsdgrad_op[_qp] * _grad_phi[_j][_qp];
  ;

  // Set the Jacobian
  RealGradient jac1 = _eps[_qp] * _deps[_qp] * dv;
  RealGradient jac2 = _deps[_qp] * depsdop_i * v;
  RealGradient jac3 = _eps[_qp] * ddepsdop_i * v;

  return _L[_qp] * (jac1 + jac2 + jac3);
}

Real
ACInterfaceKobayashi1::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // Set modified gradient vector
  const RealGradient v(-_grad_u[_qp](1), _grad_u[_qp](0), 0);

  // Set off-diagonal jaocbian terms from mobility dependence
  Real dsum =
      _L[_qp] * (_deps[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * v * _grad_test[_i][_qp]);
  dsum +=
      _L[_qp] * (_eps[_qp] * (*_ddepsdarg[cvar])[_qp] * _phi[_j][_qp] * v * _grad_test[_i][_qp]);
  dsum += (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * _eps[_qp] * _deps[_qp] * v * _grad_test[_i][_qp];

  return dsum;
}
