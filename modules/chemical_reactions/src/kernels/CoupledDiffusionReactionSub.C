//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledDiffusionReactionSub.h"

using libMesh::RealGradient;

registerMooseObject("ChemicalReactionsApp", CoupledDiffusionReactionSub);

InputParameters
CoupledDiffusionReactionSub::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>(
      "weight",
      1.0,
      "Weight of equilibrium species concentration in the primary species concentration");
  params.addCoupledVar(
      "log_k", 0.0, "Equilibrium constant of the equilbrium reaction in dissociation form");
  params.addParam<Real>("sto_u",
                        1.0,
                        "Stoichiometric coef of the primary species this kernel "
                        "operates on in the equilibrium reaction");
  params.addCoupledVar(
      "gamma_u", 1.0, "Activity coefficient of primary species that this kernel operates on");
  params.addParam<std::vector<Real>>(
      "sto_v", {}, "The stoichiometric coefficients of coupled primary species");
  params.addCoupledVar("v", "List of coupled primary species in this equilibrium species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addCoupledVar("gamma_eq", 1.0, "Activity coefficient of this equilibrium species");
  params.addClassDescription("Diffusion of equilibrium species");
  return params;
}

CoupledDiffusionReactionSub::CoupledDiffusionReactionSub(const InputParameters & parameters)
  : Kernel(parameters),
    _diffusivity(getMaterialProperty<Real>("diffusivity")),
    _weight(getParam<Real>("weight")),
    _log_k(coupledValue("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _vars(coupledIndices("v")),
    _vals(coupledValues("v")),
    _grad_vals(coupledGradients("v")),
    _gamma_u(coupledValue("gamma_u")),
    _gamma_v(isCoupled("gamma_v")
                 ? coupledValues("gamma_v") // have value
                 : std::vector<const VariableValue *>(coupledComponents("v"),
                                                      &coupledValue("gamma_v"))), // default
    _gamma_eq(coupledValue("gamma_eq"))
{
  const unsigned int n = coupledComponents("v");

  // Check that the correct number of coupled values have been provided
  if (_sto_v.size() != n)
    mooseError("The number of stoichiometric coefficients in sto_v is not equal to the number of "
               "coupled species in ",
               _name);

  if (isCoupled("gamma_v"))
    if (coupledComponents("gamma_v") != n)
      mooseError("The number of activity coefficients in gamma_v is not equal to the number of "
                 "coupled species in ",
                 _name);
}

Real
CoupledDiffusionReactionSub::computeQpResidual()
{
  RealGradient diff1 =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  for (unsigned int i = 0; i < _vals.size(); ++i)
    diff1 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

  RealGradient diff2_sum(0.0, 0.0, 0.0);
  const Real d_val = std::pow(_gamma_u[_qp] * _u[_qp], _sto_u);
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    RealGradient diff2 = d_val * _sto_v[i] * (*_gamma_v[i])[_qp] *
                         std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                         (*_grad_vals[i])[_qp];

    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        diff2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

    diff2_sum += diff2;
  }

  mooseAssert(_gamma_eq[_qp] > 0.0, "Activity coefficient must be greater than zero");
  return _weight * std::pow(10.0, _log_k[_qp]) * _diffusivity[_qp] * _grad_test[_i][_qp] *
         (diff1 + diff2_sum) / _gamma_eq[_qp];
  ;
}

Real
CoupledDiffusionReactionSub::computeQpJacobian()
{
  RealGradient diff1_1 =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_phi[_j][_qp];
  RealGradient diff1_2 = _phi[_j][_qp] * _sto_u * (_sto_u - 1.0) * _gamma_u[_qp] * _gamma_u[_qp] *
                         std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 2.0) * _grad_u[_qp];
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    diff1_1 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
    diff1_2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
  }

  RealGradient diff1 = diff1_1 + diff1_2;
  Real d_val =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _phi[_j][_qp];
  RealGradient diff2_sum(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    RealGradient diff2 = d_val * _sto_v[i] * (*_gamma_v[i])[_qp] *
                         std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                         (*_grad_vals[i])[_qp];
    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        diff2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

    diff2_sum += diff2;
  }

  return _weight * std::pow(10.0, _log_k[_qp]) * _diffusivity[_qp] * _grad_test[_i][_qp] *
         (diff1 + diff2_sum) / _gamma_eq[_qp];
}

Real
CoupledDiffusionReactionSub::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If no coupled species, return 0
  if (_vals.size() == 0)
    return 0.0;

  // If jvar is not one of the coupled species, return 0
  if (std::find(_vars.begin(), _vars.end(), jvar) == _vars.end())
    return 0.0;

  RealGradient diff1 =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    if (jvar == _vars[i])
      diff1 *= _sto_v[i] * (*_gamma_v[i])[_qp] *
               std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    else
      diff1 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
  }

  Real val_u = std::pow(_gamma_u[_qp] * _u[_qp], _sto_u);

  RealGradient diff2_1(1.0, 1.0, 1.0);
  RealGradient diff2_2(1.0, 1.0, 1.0);

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      diff2_1 = _sto_v[i] * (_sto_v[i] - 1.0) * (*_gamma_v[i])[_qp] * (*_gamma_v[i])[_qp] *
                std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 2.0) * _phi[_j][_qp] *
                (*_grad_vals[i])[_qp];
      diff2_2 = _sto_v[i] * (*_gamma_v[i])[_qp] *
                std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                _grad_phi[_j][_qp];
    }

  RealGradient diff2 = val_u * (diff2_1 + diff2_2);

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar != _vars[i])
      diff2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

  RealGradient diff3;
  RealGradient diff3_sum(0.0, 0.0, 0.0);
  Real val_jvar = 0.0;
  unsigned int var = 0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      var = i;
      val_jvar = val_u * _sto_v[i] * (*_gamma_v[i])[_qp] *
                 std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    }

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (i != var)
    {
      diff3 = val_jvar * _sto_v[i] * (*_gamma_v[i])[_qp] *
              std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
              (*_grad_vals[i])[_qp];

      for (unsigned int j = 0; j < _vals.size(); ++j)
        if (j != var && j != i)
          diff3 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

      diff3_sum += diff3;
    }

  return _weight * std::pow(10.0, _log_k[_qp]) * _diffusivity[_qp] * _grad_test[_i][_qp] *
         (diff1 + diff2 + diff3_sum) / _gamma_eq[_qp];
}
