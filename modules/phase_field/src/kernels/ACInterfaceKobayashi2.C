/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterfaceKobayashi2.h"

template<>
InputParameters validParams<ACInterfaceKobayashi2>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Anisotropic Gradient energy Allen-Cahn Kernel Part 2");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

ACInterfaceKobayashi2::ACInterfaceKobayashi2(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name"))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", _coupled_moose_vars[i]->name());
  }
}

RealGradient
ACInterfaceKobayashi2::precomputeQpResidual()
{
  // Set interfacial part of residual
  return  _eps[_qp] * _eps[_qp] *  _L[_qp] * _grad_u[_qp];
}

RealGradient
ACInterfaceKobayashi2::precomputeQpJacobian()
{
  Real cos_AC2;
  Real angle_AC2;
  Real depsdop_AC2;

  // Set the value in special situation
  if (_grad_u[_qp] * _grad_u[_qp] == 0.0)
    depsdop_AC2 = 0.0;
  else
  {
    // Define the angle between grad_u and x axis
    cos_AC2 = _grad_u[_qp](0) / std::sqrt(_grad_u[_qp] * _grad_u[_qp]);
    angle_AC2 = std::acos(cos_AC2);

    if (cos_AC2 * cos_AC2 == 1.0)
      depsdop_AC2 = 0;
    else
      depsdop_AC2 = 6.0 * 0.01 * 0.04 * -1.0 * std::sin(6.0 * (angle_AC2 - libMesh::pi/2.0)) *
                    -1.0 / std::sqrt(1.0 - cos_AC2 * cos_AC2) * (
                      _grad_phi[_j][_qp](0) * std::sqrt(_grad_u[_qp]*_grad_u[_qp]) -
                      _grad_u[_qp](0) * _grad_phi[_j][_qp] * _grad_u[_qp] / std::sqrt(_grad_u[_qp] * _grad_u[_qp])
                    ) / (_grad_u[_qp] * _grad_u[_qp]);
  }

  // Set Jacobian using product rule
  return _L[_qp] * (_eps[_qp] * _eps[_qp] * _grad_phi[_j][_qp] +
                    2.0 * _eps[_qp] * depsdop_AC2 * _phi[_j][_qp] * _grad_u[_qp]);
}

Real
ACInterfaceKobayashi2::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
  return _L[_qp] * 2.0 *_eps[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}
