//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarDamageBase.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<ScalarDamageBase>()
{
  InputParameters params = validParams<DamageBase>();
  params.addClassDescription("Base class for damage model based on a scalar damage parameter");
  params.addParam<bool>(
      "use_old_damage",
      false,
      "Whether to use the damage index from the previous step in the stress computation");
  params.addRangeCheckedParam<Real>(
      "residual_stiffness_fraction",
      1.e-8,
      "residual_stiffness_fraction>=0 & residual_stiffness_fraction<1",
      "Minimum fraction of original material stiffness retained for fully "
      "damaged material (when damage_index=1)");
  params.addRangeCheckedParam<Real>(
      "maximum_damage_increment",
      0.1,
      "maximum_damage_increment>0 & maximum_damage_increment<1",
      "maximum damage increment allowed for simulations with adaptive time step");
  params.addParam<std::string>("damage_index_name",
                               "damage_index",
                               "name of the material property where the damage index is stored");
  return params;
}

ScalarDamageBase::ScalarDamageBase(const InputParameters & parameters)
  : DamageBase(parameters),
    _damage_index_name(getParam<std::string>("damage_index_name")),
    _damage_index(declareProperty<Real>(_base_name + _damage_index_name)),
    _damage_index_old(getMaterialPropertyOld<Real>(_base_name + _damage_index_name)),
    _damage_index_older(getMaterialPropertyOlder<Real>(_base_name + _damage_index_name)),
    _use_old_damage(getParam<bool>("use_old_damage")),
    _residual_stiffness_fraction(getParam<Real>("residual_stiffness_fraction")),
    _maximum_damage_increment(getParam<Real>("maximum_damage_increment"))
{
}

void
ScalarDamageBase::initQpStatefulProperties()
{
  _damage_index[_qp] = 0.0;
}

const Real &
ScalarDamageBase::getQpDamageIndex(unsigned int qp)
{
  setQp(qp);
  updateQpDamageIndex();
  return _damage_index[_qp];
}

void
ScalarDamageBase::updateDamage()
{
  updateQpDamageIndex();
}

void
ScalarDamageBase::updateStressForDamage(RankTwoTensor & stress_new)
{
  // Avoid multiplying by a small negative number, which could occur if damage_index
  // is slightly greater than 1.0
  stress_new *=
      std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp] : _damage_index[_qp])), 0.0);
}

void
ScalarDamageBase::computeUndamagedOldStress(RankTwoTensor & stress_old)
{
  Real damage_index_old =
      std::max((1.0 - (_use_old_damage ? _damage_index_older[_qp] : _damage_index_old[_qp])), 0.0);

  if (damage_index_old > 0.0)
    stress_old /= damage_index_old;
}

void
ScalarDamageBase::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  jacobian_mult *= std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp] : _damage_index[_qp])),
                            _residual_stiffness_fraction);
}

Real
ScalarDamageBase::computeTimeStepLimit()
{
  Real current_damage_increment = (_damage_index[_qp] - _damage_index_old[_qp]);
  if (MooseUtils::absoluteFuzzyEqual(current_damage_increment, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _maximum_damage_increment / current_damage_increment;
}
