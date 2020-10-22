//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteStrainElasticStress.h"

registerMooseObject("TensorMechanicsApp", ADComputeFiniteStrainElasticStress);

InputParameters
ADComputeFiniteStrainElasticStress::validParams()
{
  InputParameters params = ADComputeStressBase::validParams();
  params.addClassDescription("Compute stress using elasticity for finite strains");
  params.addParam<bool>(
      "use_old_elasticity_tensor",
      false,
      "Flag to optionally use the elasticity tensor computed at the previous timestep.");
  return params;
}

ADComputeFiniteStrainElasticStress::ADComputeFiniteStrainElasticStress(
    const InputParameters & parameters)
  : ADComputeStressBase(parameters),
    GuaranteeConsumer(this),
    _use_old_elasticity_tensor(getParam<bool>("use_old_elasticity_tensor")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(
        _use_old_elasticity_tensor
            ? getGenericZeroMaterialProperty<RankFourTensor, true>("zero")
            : getGenericMaterialProperty<RankFourTensor, true>(_elasticity_tensor_name)),
    _elasticity_tensor_old(_use_old_elasticity_tensor
                               ? getMaterialPropertyOld<RankFourTensor>(_elasticity_tensor_name)
                               : getZeroMaterialProperty<RankFourTensor>("zero")),
    _strain_increment(getADMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getADMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

void
ADComputeFiniteStrainElasticStress::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError(
        "ADComputeFiniteStrainElasticStress can only be used with elasticity tensor materials "
        "that guarantee isotropic tensors.");
}

void
ADComputeFiniteStrainElasticStress::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  ADRankTwoTensor intermediate_stress;

  if (_use_old_elasticity_tensor)
    intermediate_stress =
        _elasticity_tensor_old[_qp] * (_strain_increment[_qp] + _elastic_strain_old[_qp]);
  else
    intermediate_stress =
        _elasticity_tensor[_qp] * (_strain_increment[_qp] + _elastic_strain_old[_qp]);

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}
