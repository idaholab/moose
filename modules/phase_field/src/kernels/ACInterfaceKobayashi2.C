/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterfaceKobayashi2.h"

template <>
InputParameters
validParams<ACInterfaceKobayashi2>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Anisotropic Gradient energy Allen-Cahn Kernel Part 2");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addParam<MaterialPropertyName>(
      "depsdgrad_op_name",
      "depsdgrad_op",
      "The derivative of the anisotropic interface parameter eps with respect to grad_op");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

ACInterfaceKobayashi2::ACInterfaceKobayashi2(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name")),
    _depsdgrad_op(getMaterialProperty<RealGradient>("depsdgrad_op_name"))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    const VariableName iname = _coupled_moose_vars[i]->name();
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", iname);
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", iname);
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
