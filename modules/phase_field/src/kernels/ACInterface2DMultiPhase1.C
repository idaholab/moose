//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACInterface2DMultiPhase1.h"

registerMooseObject("PhaseFieldApp", ACInterface2DMultiPhase1);

InputParameters
ACInterface2DMultiPhase1::validParams()
{
  InputParameters params = ACInterface::validParams();
  params.addClassDescription(
      "Gradient energy Allen-Cahn Kernel where the derivative of interface parameter kappa "
      "wrt the gradient of order parameter is considered.");
  params.addParam<MaterialPropertyName>("dkappadgrad_etaa_name",
                                        "dkappadgrad_etaa",
                                        "The derivative of the kappa with respect to grad_etaa");
  params.addParam<MaterialPropertyName>(
      "d2kappadgrad_etaa_name",
      "d2kappadgrad_etaa",
      "The second derivative of the kappa with respect to grad_etaa");
  params.addRequiredCoupledVar(
      "etas", "All other coupled order parameters eta_i of the multiphase problem");
  return params;
}

ACInterface2DMultiPhase1::ACInterface2DMultiPhase1(const InputParameters & parameters)
  : ACInterface(parameters),
    _dkappadgrad_etaa(getMaterialProperty<RealGradient>("dkappadgrad_etaa_name")),
    _d2kappadgrad_etaa(getMaterialProperty<RealTensorValue>("d2kappadgrad_etaa_name")),
    _num_etas(coupledComponents("etas")),
    _eta(_num_etas),
    _grad_eta(_num_etas)
{
}

Real
ACInterface2DMultiPhase1::sumSquareGradEta()
{
  // get the sum of square of gradients of all order parameters
  Real SumSquareGradOp = _grad_u[_qp] * _grad_u[_qp];
  for (unsigned int i = 0; i < _num_etas; ++i)
  {
    _grad_eta[i] = &coupledGradient("etas", i);
    SumSquareGradOp += (*_grad_eta[i])[_qp] * (*_grad_eta[i])[_qp];
  }
  return SumSquareGradOp;
}

Real
ACInterface2DMultiPhase1::computeQpResidual()
{
  return 0.5 * nablaLPsi() * _dkappadgrad_etaa[_qp] * sumSquareGradEta();
}

Real
ACInterface2DMultiPhase1::computeQpJacobian()
{
  // dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla (L\psi) \right) \f$
  RealGradient dsum = _dLdop[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];

  // compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL =
        _grad_phi[_j][_qp] * _dLdop[_qp] + _grad_u[_qp] * _phi[_j][_qp] * _d2Ldop2[_qp];

    for (unsigned int i = 0; i < _n_args; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[i])[_qp];

    dsum += dgradL * _test[_i][_qp];
  }
  Real jac1 = 0.5 * dsum * _dkappadgrad_etaa[_qp] * sumSquareGradEta();
  Real jac2 =
      0.5 * nablaLPsi() * (_d2kappadgrad_etaa[_qp] * _grad_phi[_j][_qp]) * sumSquareGradEta();
  Real jac3 = nablaLPsi() * _dkappadgrad_etaa[_qp] * _grad_u[_qp] * _grad_phi[_j][_qp];
  return jac1 + jac2 + jac3;
}

Real
ACInterface2DMultiPhase1::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla (L\psi) \right) \f$
  RealGradient dsum = (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];

  // compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL = _grad_phi[_j][_qp] * (*_dLdarg[cvar])[_qp] +
                          _grad_u[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[cvar])[_qp];

    for (unsigned int i = 0; i < _n_args; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldarg2[cvar][i])[_qp];

    dsum += dgradL * _test[_i][_qp];
  }

  // the gradient of coupled variable etab
  _grad_eta[0] = &coupledGradient("etas", 0);

  Real jac1 = 0.5 * dsum * _dkappadgrad_etaa[_qp] * sumSquareGradEta();
  Real jac2 =
      -0.5 * nablaLPsi() * (_d2kappadgrad_etaa[_qp] * _grad_phi[_j][_qp]) * sumSquareGradEta();
  Real jac3 = nablaLPsi() * _dkappadgrad_etaa[_qp] * (*_grad_eta[0])[_qp] * _grad_phi[_j][_qp];
  return jac1 + jac2 + jac3;
}
