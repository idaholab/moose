/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterfaceKobayashi1.h"

template<>
InputParameters validParams<ACInterfaceKobayashi1>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Anisotropic gradient energy Allen-Cahn Kernel Part 1");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addParam<MaterialPropertyName>("eps1_name", "eps1", "The derivative of anisotropic parameter with respect to angle");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

ACInterfaceKobayashi1::ACInterfaceKobayashi1(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name")),
    _eps1(getMaterialProperty<Real>("eps1_name"))

{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);
  _deps1darg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", _coupled_moose_vars[i]->name());
    _deps1darg[i] = &getMaterialPropertyDerivative<Real>("eps1_name", _coupled_moose_vars[i]->name());
  }
}

RealGradient
ACInterfaceKobayashi1::precomputeQpResidual()
{
  // Set interfacial part of residual
  RealVectorValue v = _eps[_qp]* _eps1[_qp] * _L[_qp] * _grad_u[_qp];
  std::swap(v(0), v(1));
  v(0) *= -1.0;
  return v;
}

RealGradient
ACInterfaceKobayashi1::precomputeQpJacobian()
{
  Real cos_AC1;
  Real depsdop_AC1;
  Real deps1dop_AC1;
  Real angle_AC1;

  // Set the value in special situation
  if (_grad_u[_qp]*_grad_u[_qp] == 0)
  {
    depsdop_AC1 = 0;
    deps1dop_AC1 = 0;
  }
  // Define the angle between grad_u and x aixs
  else
  {
    cos_AC1 = _grad_u[_qp](0) / std::sqrt(_grad_u[_qp] * _grad_u[_qp]);
    angle_AC1 = std::acos(cos_AC1);

    // Set the value in special situation
    if (cos_AC1 * cos_AC1 == 1)
    {
      depsdop_AC1 = 0.0;
      deps1dop_AC1 = 0.0;
    }
    else
    {
      // Derivative of eps with respect to u
      depsdop_AC1 = 6*0.01*0.04*(-1) * std::sin(6*(angle_AC1 - libMesh::pi/2.0)) *
                    -1.0 / std::sqrt(1.0 - cos_AC1 * cos_AC1) *
                    ( _grad_phi[_j][_qp](0) * std::sqrt(_grad_u[_qp] * _grad_u[_qp]) -
                      _grad_u[_qp](0) * _grad_phi[_j][_qp] * _grad_u[_qp] / std::sqrt(_grad_u[_qp] * _grad_u[_qp])
                    ) / (_grad_u[_qp] * _grad_u[_qp]);

      // Derivative of eps1 with respect to u
      deps1dop_AC1 = 36*0.01*0.04*(-1) * std::cos(6.0 * (angle_AC1 - libMesh::pi/2.0)) *
                     -1/std::sqrt(1.0 - cos_AC1 * cos_AC1) *
                     ( _grad_phi[_j][_qp](0) * std::sqrt(_grad_u[_qp] * _grad_u[_qp]) -
                       _grad_u[_qp](0) * _grad_phi[_j][_qp] * _grad_u[_qp] / std::sqrt(_grad_u[_qp] * _grad_u[_qp])
                     ) / (_grad_u[_qp] * _grad_u[_qp]);
    }
  }

  // Set the Jacobian
  RealVectorValue v = _L[_qp] * (_eps[_qp]*_eps1[_qp]* _grad_phi[_j][_qp] +
                                 depsdop_AC1 * _eps1[_qp] * _phi[_j][_qp] * _grad_u[_qp] +
                                 deps1dop_AC1 * _eps[_qp] * _phi[_j][_qp] * _grad_u[_qp]);
  std::swap(v(0), v(1));
  v(0) *= -1.0;
  return v;
}

Real
ACInterfaceKobayashi1::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
  return _L[_qp] * (_eps1[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp] +
                    _eps[_qp] * (*_deps1darg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp]);
}
