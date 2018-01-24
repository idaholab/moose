//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RateDepSmearIsoCrackModel.h"

template <>
InputParameters
validParams<RateDepSmearIsoCrackModel>()
{

  InputParameters params = validParams<RateDepSmearCrackModel>();

  params.addRequiredParam<Real>("critical_energy", "Critical Energy");
  params.addParam<Real>("k_fail", 1e-6, "Post failure stiffness");
  params.addParam<Real>("upper_limit_damage",
                        5.0,
                        "Upper limit of damage beyond which constitutive check is not performed");

  return params;
}

RateDepSmearIsoCrackModel::RateDepSmearIsoCrackModel(const InputParameters & parameters)
  : RateDepSmearCrackModel(parameters),
    _crit_energy(getParam<Real>("critical_energy")),
    _kfail(getParam<Real>("k_fail")),
    _upper_lim_damage(getParam<Real>("upper_limit_damage")),
    _energy(declareProperty<Real>("energy")),
    _energy_old(getMaterialPropertyOld<Real>("energy"))
{

  if (_nstate != 2)
    mooseError(" RateDpeSmearIsoCrackModel Error: Requires 2 state variables - nstate = 2");
}

void
RateDepSmearIsoCrackModel::initQpStatefulProperties()
{
  RateDepSmearCrackModel::initQpStatefulProperties();

  _intvar[_qp][1] = const_cast<MaterialProperty<std::vector<Real>> &>(_intvar_old)[_qp][1] =
      _crit_energy;

  _energy[_qp] = 0.0;
}

void
RateDepSmearIsoCrackModel::initVariables()
{

  RateDepSmearCrackModel::initVariables();

  _stress0.columnMajorMatrix().eigen(_s0_diag, _s0_evec);

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _s0_diag_pos(i, i) = (std::abs(_s0_diag(i, 0)) + _s0_diag(i, 0)) / 2.0;
    _s0_diag_neg(i, i) = (std::abs(_s0_diag(i, 0)) - _s0_diag(i, 0)) / 2.0;
  }

  _dstrain.columnMajorMatrix().eigen(_dstrain_diag, _dstrain_evec);

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _dstrain_diag_pos(i, i) = (std::abs(_dstrain_diag(i, 0)) + _dstrain_diag(i, 0)) / 2.0;
    _dstrain_diag_neg(i, i) = (std::abs(_dstrain_diag(i, 0)) - _dstrain_diag(i, 0)) / 2.0;
  }
}

Real
RateDepSmearIsoCrackModel::damageRate()
{

  Real den = 0.0;

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    den += _s0_diag_pos(i, i) * _dstrain_diag_pos(i, i);

  _energy[_qp] = _energy_old[_qp] + den;

  Real e = _energy[_qp];

  Real fac = e - _intvar_tmp[1];
  Real val = 0.0;

  _ddamagerate_drs = 0.0;

  if (fac > 0.0)
  {
    val = std::pow(fac, _exponent);
    val *= _ref_damage_rate * _dt;

    Real dval = -_exponent * std::pow(fac, _exponent - 1.0);

    _ddamagerate_drs = dval * _ref_damage_rate * _dt;
  }

  return val;
}

void
RateDepSmearIsoCrackModel::calcStateIncr()
{

  Real val = damageRate();

  if (std::abs(_intvar_old_tmp[0]) < _zero_tol && std::abs(val) > _intvar_incr_tol)
  {
    _err_tol = true;
    return;
  }

  if (std::abs(_intvar_old_tmp[0]) > _zero_tol &&
      std::abs(_intvar_old_tmp[0]) < _upper_lim_damage &&
      std::abs(val) > _intvar_incr_tol * std::abs(_intvar_old_tmp[0]))
  {
    _err_tol = true;
    return;
  }

  for (unsigned int i = 0; i < _nstate; i++)
    _intvar_incr[i] = val;
}

void
RateDepSmearIsoCrackModel::calcJacobian()
{

  _jac[0] = 1.0;
  _jac[1] = -_ddamagerate_drs;
  _jac[2] = 0.0;
  _jac[3] = 1 - _ddamagerate_drs;
}

void
RateDepSmearIsoCrackModel::postSolveStress()
{
  ColumnMajorMatrix stress_cmm = _s0_diag_pos * (std::exp(-_intvar_tmp[0]) + _kfail) - _s0_diag_neg;
  _stress_new = _s0_evec * stress_cmm * _s0_evec.transpose();
}

RateDepSmearIsoCrackModel::~RateDepSmearIsoCrackModel() {}
