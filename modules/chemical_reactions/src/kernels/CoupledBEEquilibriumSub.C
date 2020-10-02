//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledBEEquilibriumSub.h"

registerMooseObject("ChemicalReactionsApp", CoupledBEEquilibriumSub);

InputParameters
CoupledBEEquilibriumSub::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addParam<Real>(
      "weight", 1.0, "The weight of the equilibrium species in total concentration");
  params.addCoupledVar("log_k", 0.0, "The equilibrium constant of this equilibrium species");
  params.addParam<Real>(
      "sto_u",
      1.0,
      "The stoichiometric coefficient of the primary species this kernel operates on");
  params.addCoupledVar(
      "gamma_u", 1.0, "Activity coefficient of primary species that this kernel operates on");
  params.addParam<std::vector<Real>>("sto_v",
                                     "The stoichiometric coefficients of coupled primary species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addCoupledVar("gamma_eq", 1.0, "Activity coefficient of this equilibrium species");
  params.addCoupledVar("v", "Coupled primary species constituting the equilibrium species");
  params.addClassDescription("Derivative of equilibrium species concentration wrt time");
  return params;
}

CoupledBEEquilibriumSub::CoupledBEEquilibriumSub(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _weight(getParam<Real>("weight")),
    _log_k(coupledValue("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _gamma_u(coupledValue("gamma_u")),
    _gamma_u_old(coupledValueOld("gamma_u")),
    _gamma_v(isCoupled("gamma_v")
                 ? coupledValues("gamma_v") // have value
                 : std::vector<const VariableValue *>(coupledComponents("v"),
                                                      &coupledValue("gamma_v"))), // default
    _gamma_v_old(isCoupled("gamma_v")
                     ? coupledValuesOld("gamma_v") // have value
                     : std::vector<const VariableValue *>(coupledComponents("v"),
                                                          &coupledValue("gamma_v"))), // default
    _gamma_eq(coupledValue("gamma_eq")),
    _gamma_eq_old(coupledValueOld("gamma_eq")),
    _porosity(getMaterialProperty<Real>("porosity")),
    _vars(coupledIndices("v")),
    _v_vals(coupledValues("v")),
    _v_vals_old(coupledValuesOld("v")),
    _u_old(valueOld())
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
CoupledBEEquilibriumSub::computeQpResidual()
{
  mooseAssert(_gamma_eq[_qp] > 0.0, "Activity coefficient must be greater than zero");

  // Contribution due to primary species that this kernel acts on
  Real val_new =
      std::pow(10.0, _log_k[_qp]) * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u) / _gamma_eq[_qp];
  Real val_old = std::pow(10.0, _log_k[_qp]) * std::pow(_gamma_u_old[_qp] * _u_old[_qp], _sto_u) /
                 _gamma_eq_old[_qp];

  // Contribution due to coupled primary species
  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    val_new *= std::pow((*_gamma_v[i])[_qp] * (*_v_vals[i])[_qp], _sto_v[i]);
    val_old *= std::pow((*_gamma_v_old[i])[_qp] * (*_v_vals_old[i])[_qp], _sto_v[i]);
  }

  return _porosity[_qp] * _weight * _test[_i][_qp] * (val_new - val_old) / _dt;
}

Real
CoupledBEEquilibriumSub::computeQpJacobian()
{
  Real val_new = std::pow(10.0, _log_k[_qp]) * _sto_u * _gamma_u[_qp] *
                 std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _phi[_j][_qp] / _gamma_eq[_qp];

  for (unsigned int i = 0; i < _vars.size(); ++i)
    val_new *= std::pow((*_gamma_v[i])[_qp] * (*_v_vals[i])[_qp], _sto_v[i]);

  return _porosity[_qp] * _test[_i][_qp] * _weight * val_new / _dt;
}

Real
CoupledBEEquilibriumSub::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If no coupled species, return 0
  if (_v_vals.size() == 0)
    return 0.0;

  // If jvar is not one of the coupled species, return 0
  if (std::find(_vars.begin(), _vars.end(), jvar) == _vars.end())
    return 0.0;

  Real val_new =
      std::pow(10.0, _log_k[_qp]) * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u) / _gamma_eq[_qp];

  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    if (jvar == _vars[i])
      val_new *= _sto_v[i] * (*_gamma_v[i])[_qp] *
                 std::pow((*_gamma_v[i])[_qp] * (*_v_vals[i])[_qp], _sto_v[i] - 1.0) *
                 _phi[_j][_qp];
    else
      val_new *= std::pow((*_gamma_v[i])[_qp] * (*_v_vals[i])[_qp], _sto_v[i]);
  }

  return _porosity[_qp] * _test[_i][_qp] * _weight * val_new / _dt;
}
