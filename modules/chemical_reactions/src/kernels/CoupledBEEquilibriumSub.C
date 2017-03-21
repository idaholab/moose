/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledBEEquilibriumSub.h"

template <>
InputParameters
validParams<CoupledBEEquilibriumSub>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("weight", 1.0, "The weight of the equilibrium species");
  params.addParam<Real>(
      "log_k",
      0.0,
      "The equilibrium constant of this equilibrium species in the dissociation reaction");
  params.addParam<Real>(
      "sto_u",
      1.0,
      "The stoichiometric coefficient of the primary variable this kernel operates on");
  params.addRequiredParam<std::vector<Real>>(
      "sto_v", "The stoichiometric coefficients of coupled primary species");
  params.addCoupledVar("v", "Coupled primary species constituting the equilibrium species");
  return params;
}

CoupledBEEquilibriumSub::CoupledBEEquilibriumSub(const InputParameters & parameters)
  : Kernel(parameters),
    _weight(getParam<Real>("weight")),
    _log_k(getParam<Real>("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _porosity(getMaterialProperty<Real>("porosity")),
    _u_old(valueOld())
{
  const unsigned int n = coupledComponents("v");
  _vars.resize(n);
  _v_vals.resize(n);
  _v_vals_old.resize(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    _vars[i] = coupled("v", i);
    _v_vals[i] = &coupledValue("v", i);
    _v_vals_old[i] = &coupledValueOld("v", i);
  }
}

Real
CoupledBEEquilibriumSub::computeQpResidual()
{
  Real _val_new = std::pow(10.0, _log_k) * std::pow(_u[_qp], _sto_u);
  Real _val_old = std::pow(10.0, _log_k) * std::pow(_u_old[_qp], _sto_u);
  for (unsigned int i = 0; i < _v_vals.size(); ++i)
  {
    _val_new *= std::pow((*_v_vals[i])[_qp], _sto_v[i]);
    _val_old *= std::pow((*_v_vals_old[i])[_qp], _sto_v[i]);
  }

  return _porosity[_qp] * _weight * _test[_i][_qp] * (_val_new - _val_old) / _dt;
}

Real
CoupledBEEquilibriumSub::computeQpJacobian()
{
  Real _val_new = std::pow(10.0, _log_k) * _sto_u * std::pow(_u[_qp], _sto_u - 1.0) * _phi[_j][_qp];
  for (unsigned int i = 0; i < _v_vals.size(); ++i)
    _val_new *= std::pow((*_v_vals[i])[_qp], _sto_v[i]);

  return _porosity[_qp] * _test[_i][_qp] * _weight * _val_new / _dt;
}

Real
CoupledBEEquilibriumSub::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real _val_new = std::pow(10.0, _log_k) * std::pow(_u[_qp], _sto_u);

  if (_vars.size() == 0)
    return 0.0;

  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    if (jvar == _vars[i])
      _val_new *= _sto_v[i] * std::pow((*_v_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    else
      _val_new *= std::pow((*_v_vals[i])[_qp], _sto_v[i]);
  }

  return _porosity[_qp] * _test[_i][_qp] * _weight * _val_new / _dt;
}
