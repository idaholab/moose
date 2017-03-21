/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterfaceKobayashi1.h"

template <>
InputParameters
validParams<ACInterfaceKobayashi1>()
{
  InputParameters params = validParams<KernelGrad>();
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
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
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
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);
  _ddepsdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    const VariableName iname = _coupled_moose_vars[i]->name();
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", iname);
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", iname);
    _ddepsdarg[i] = &getMaterialPropertyDerivative<Real>("deps_name", iname);
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
