//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarMaterialDamage.h"
#include "MooseUtils.h"

registerMooseObject("TensorMechanicsApp", ScalarMaterialDamage);

template <>
InputParameters
validParams<ScalarMaterialDamage>()
{
  InputParameters params = validParams<DamageBase>();
  params.addClassDescription(
      "Scalar damage model for which the damage is prescribed by another material");
  params.addParam<bool>(
      "use_old_damage",
      false,
      "Whether to use the damage index from the previous step in the stress computation");
  params.addRequiredParam<MaterialPropertyName>("damage_index",
                                                "Name of the material property containing the "
                                                "damage index, which goes from 0 (undamaged) to 1 "
                                                "(fully damaged)");
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
      "maximum damage increment allowed for simulations with adaptative time step");
  return params;
}

ScalarMaterialDamage::ScalarMaterialDamage(const InputParameters & parameters)
  : DamageBase(parameters),
    _use_old_damage(getParam<bool>("use_old_damage")),
    _damage_index(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("damage_index"))),
    _damage_index_old(
        getMaterialPropertyOldByName<Real>(getParam<MaterialPropertyName>("damage_index"))),
    _residual_stiffness_fraction(getParam<Real>("residual_stiffness_fraction")),
    _maximum_damage_increment(getParam<Real>("maximum_damage_increment"))
{
}

void
ScalarMaterialDamage::updateDamage()
{
  if (MooseUtils::absoluteFuzzyLessThan(_damage_index[_qp], 0.0) ||
      MooseUtils::absoluteFuzzyGreaterThan(_damage_index[_qp], 1.0))
    mooseError("damage_index must be between 0 and 1. Current value is: ", _damage_index[_qp]);
}

void
ScalarMaterialDamage::updateStressForDamage(RankTwoTensor & stress_new)
{
  // Avoid multiplying by a small negative number, which could occur if damage_index
  // is slightly greater than 1.0
  stress_new *=
      std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp] : _damage_index[_qp])), 0.0);
}

void
ScalarMaterialDamage::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  jacobian_mult *= std::max((1.0 - (_use_old_damage ? _damage_index_old[_qp] : _damage_index[_qp])),
                            _residual_stiffness_fraction);
}

Real
ScalarMaterialDamage::computeTimeStepLimit()
{
  Real current_damage_increment = (_damage_index[_qp] - _damage_index_old[_qp]);
  if (MooseUtils::absoluteFuzzyEqual(current_damage_increment, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _maximum_damage_increment / current_damage_increment;
}
