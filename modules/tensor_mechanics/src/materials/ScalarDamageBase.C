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

template <bool is_ad>
InputParameters
ScalarDamageBaseTempl<is_ad>::validParams()
{
  InputParameters params = DamageBaseTempl<is_ad>::validParams();
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
  params.addRangeCheckedParam<Real>("maximum_damage",
                                    1.0,
                                    "maximum_damage>=0 & maximum_damage<=1",
                                    "Maximum value allowed for damage index");
  params.addParam<MaterialPropertyName>(
      "damage_index_name",
      "damage_index",
      "name of the material property where the damage index is stored");
  return params;
}

template <bool is_ad>
ScalarDamageBaseTempl<is_ad>::ScalarDamageBaseTempl(const InputParameters & parameters)
  : DamageBaseTempl<is_ad>(parameters),
    _damage_index_name(this->template getParam<MaterialPropertyName>("damage_index_name")),
    _damage_index(
        this->template declareGenericPropertyByName<Real, is_ad>(_base_name + _damage_index_name)),
    _damage_index_old(this->template getMaterialPropertyOld<Real>(_base_name + _damage_index_name)),
    _damage_index_older(
        this->template getMaterialPropertyOlder<Real>(_base_name + _damage_index_name)),
    _use_old_damage(this->template getParam<bool>("use_old_damage")),
    _residual_stiffness_fraction(this->template getParam<Real>("residual_stiffness_fraction")),
    _maximum_damage_increment(this->template getParam<Real>("maximum_damage_increment")),
    _maximum_damage(this->template getParam<Real>("maximum_damage"))
{
}

template <bool is_ad>
void
ScalarDamageBaseTempl<is_ad>::initQpStatefulProperties()
{
  _damage_index[_qp] = 0.0;
}

template <bool is_ad>
const GenericReal<is_ad> &
ScalarDamageBaseTempl<is_ad>::getQpDamageIndex(unsigned int qp)
{
  setQp(qp);
  updateDamage();
  return _damage_index[_qp];
}

template <bool is_ad>
void
ScalarDamageBaseTempl<is_ad>::updateDamage()
{
  updateQpDamageIndex();
  _damage_index[_qp] = std::min(_maximum_damage, _damage_index[_qp]);
}

template <bool is_ad>
void
ScalarDamageBaseTempl<is_ad>::updateStressForDamage(GenericRankTwoTensor<is_ad> & stress_new)
{
  // Avoid multiplying by a small negative number, which could occur if damage_index
  // is slightly greater than 1.0
  stress_new *=
      std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp] : _damage_index[_qp])), 0.0);
}

template <bool is_ad>
void
ScalarDamageBaseTempl<is_ad>::computeUndamagedOldStress(RankTwoTensor & stress_old)
{
  Real damage_index_old =
      std::max((1.0 - (_use_old_damage ? _damage_index_older[_qp] : _damage_index_old[_qp])), 0.0);

  if (damage_index_old > 0.0)
    stress_old /= damage_index_old;
}

template <bool is_ad>
void
ScalarDamageBaseTempl<is_ad>::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  jacobian_mult *= std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp]
                                                    : MetaPhysicL::raw_value(_damage_index[_qp]))),
                            _residual_stiffness_fraction);
}

template <bool is_ad>
Real
ScalarDamageBaseTempl<is_ad>::computeTimeStepLimit()
{
  Real current_damage_increment =
      (MetaPhysicL::raw_value(_damage_index[_qp]) - _damage_index_old[_qp]);
  if (MooseUtils::absoluteFuzzyEqual(current_damage_increment, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _maximum_damage_increment / current_damage_increment;
}

template class ScalarDamageBaseTempl<false>;
template class ScalarDamageBaseTempl<true>;
