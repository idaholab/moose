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

InputParameters
ComputeDamageStress::validParams()
{
  InputParameters params = ComputeFiniteStrainElasticStress::validParams();
  params.addClassDescription(
      "Compute stress for damaged elastic materials in conjunction with a damage model.");
  params.addRequiredParam<MaterialName>("damage_model", "Name of the damage model");
  return params;
}

ComputeDamageStress::ComputeDamageStress(const InputParameters & parameters)
  : ComputeFiniteStrainElasticStress(parameters),
    _material_timestep_limit(declareProperty<Real>("material_timestep_limit")),
    _damage_model(nullptr)
{
}

void
ComputeDamageStress::initialSetup()
{
  MaterialName damage_model_name = getParam<MaterialName>("damage_model");
  DamageBase * dmb = dynamic_cast<DamageBase *>(&getMaterialByName(damage_model_name));
  if (dmb)
    _damage_model = dmb;
  else
    paramError("damage_model",
               "Damage Model " + damage_model_name + " is not compatible with ComputeDamageStress");
}

void
ComputeDamageStress::computeQpStress()
{
  ComputeFiniteStrainElasticStress::computeQpStress();

  _damage_model->setQp(_qp);
  _damage_model->updateDamage();
  _damage_model->updateStressForDamage(_stress[_qp]);
  _damage_model->finiteStrainRotation(_rotation_increment[_qp]);
  _damage_model->updateJacobianMultForDamage(_Jacobian_mult[_qp]);

  _material_timestep_limit[_qp] = _damage_model->computeTimeStepLimit();
}
