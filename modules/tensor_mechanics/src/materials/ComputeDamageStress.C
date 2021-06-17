//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDamageStress.h"
#include "DamageBase.h"

registerMooseObject("TensorMechanicsApp", ComputeDamageStress);
registerMooseObject("TensorMechanicsApp", ADComputeDamageStress);

template <bool is_ad>
InputParameters
ComputeDamageStressTempl<is_ad>::validParams()
{
  InputParameters params = ComputeFiniteStrainElasticStressTempl<is_ad>::validParams();
  params.addClassDescription(
      "Compute stress for damaged elastic materials in conjunction with a damage model.");
  params.addRequiredParam<MaterialName>("damage_model", "Name of the damage model");
  return params;
}

template <bool is_ad>
ComputeDamageStressTempl<is_ad>::ComputeDamageStressTempl(const InputParameters & parameters)
  : ComputeFiniteStrainElasticStressTempl<is_ad>(parameters),
    _material_timestep_limit(this->template declareProperty<Real>("material_timestep_limit")),
    _damage_model(nullptr)
{
}

template <bool is_ad>
void
ComputeDamageStressTempl<is_ad>::initialSetup()
{
  MaterialName damage_model_name = this->template getParam<MaterialName>("damage_model");
  DamageBaseTempl<is_ad> * dmb =
      dynamic_cast<DamageBaseTempl<is_ad> *>(&this->getMaterialByName(damage_model_name));
  if (dmb)
    _damage_model = dmb;
  else
    this->paramError("damage_model",
                     "Damage Model " + damage_model_name +
                         " is not compatible with ComputeDamageStress");
}

template <>
void
ComputeDamageStressTempl<false>::computeQpStress()
{
  ComputeFiniteStrainElasticStressTempl<false>::computeQpStress();

  _damage_model->setQp(_qp);
  _damage_model->updateDamage();
  _damage_model->updateStressForDamage(this->_stress[_qp]);
  _damage_model->finiteStrainRotation(this->_rotation_increment[_qp]);
  _damage_model->updateJacobianMultForDamage(_Jacobian_mult[_qp]);

  _material_timestep_limit[_qp] = _damage_model->computeTimeStepLimit();
}

template <>
void
ComputeDamageStressTempl<true>::computeQpStress()
{
  ComputeFiniteStrainElasticStressTempl<true>::computeQpStress();

  _damage_model->setQp(_qp);
  _damage_model->updateDamage();
  _damage_model->updateStressForDamage(this->_stress[_qp]);
  _damage_model->finiteStrainRotation(this->_rotation_increment[_qp]);

  _material_timestep_limit[_qp] = _damage_model->computeTimeStepLimit();
}

template class ComputeDamageStressTempl<false>;
template class ComputeDamageStressTempl<true>;
