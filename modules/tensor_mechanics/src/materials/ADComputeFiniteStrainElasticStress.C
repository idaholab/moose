//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteStrainElasticStress.h"

registerADMooseObject("TensorMechanicsApp", ADComputeFiniteStrainElasticStress);

defineADValidParams(
    ADComputeFiniteStrainElasticStress,
    ADComputeStressBase,
    params.addClassDescription("Compute stress using elasticity for finite strains"););

template <ComputeStage compute_stage>
ADComputeFiniteStrainElasticStress<compute_stage>::ADComputeFiniteStrainElasticStress(
    const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    GuaranteeConsumer(this),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adGetADMaterialProperty<RankFourTensor>(_elasticity_tensor_name)),
    _strain_increment(
        adGetADMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        adGetADMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(adGetMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(
        adGetMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

template <ComputeStage compute_stage>
void
ADComputeFiniteStrainElasticStress<compute_stage>::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError(
        "ADComputeFiniteStrainElasticStress can only be used with elasticity tensor materials "
        "that guarantee isotropic tensors.");
}

template <ComputeStage compute_stage>
void
ADComputeFiniteStrainElasticStress<compute_stage>::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  ADRankTwoTensor intermediate_stress;

  intermediate_stress =
      _elasticity_tensor[_qp] * (_strain_increment[_qp] + _elastic_strain_old[_qp]);

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}

// explicit instantiation is required for AD base classes
adBaseClass(ADComputeFiniteStrainElasticStress);
