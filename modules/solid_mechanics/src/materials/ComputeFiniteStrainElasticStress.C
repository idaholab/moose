//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFiniteStrainElasticStress.h"

registerMooseObject("TensorMechanicsApp", ComputeFiniteStrainElasticStress);

InputParameters
ComputeFiniteStrainElasticStress::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Compute stress using elasticity for finite strains");
  return params;
}

ComputeFiniteStrainElasticStress::ComputeFiniteStrainElasticStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    GuaranteeConsumer(this),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _rotation_total(declareProperty<RankTwoTensor>(_base_name + "rotation_total")),
    _rotation_total_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "rotation_total")),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

void
ComputeFiniteStrainElasticStress::initialSetup()
{
}

void
ComputeFiniteStrainElasticStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  RankTwoTensor identity_rotation(RankTwoTensor::initIdentity);

  _rotation_total[_qp] = identity_rotation;
}

void
ComputeFiniteStrainElasticStress::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  RankTwoTensor intermediate_stress;

  if (hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
  {
    intermediate_stress =
        _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + _strain_increment[_qp]);

    // Compute dstress_dstrain
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; // This is NOT the exact jacobian
  }
  else
  {
    // Rotate elasticity tensor to the intermediate configuration
    // That is, elasticity tensor is defined in the previous time step
    // This is consistent with the definition of strain increment
    // The stress is projected onto the current configuration a few lines below
    RankFourTensor elasticity_tensor_rotated = _elasticity_tensor[_qp];
    elasticity_tensor_rotated.rotate(_rotation_total_old[_qp]);

    intermediate_stress =
        elasticity_tensor_rotated * (_elastic_strain_old[_qp] + _strain_increment[_qp]);

    // Update current total rotation matrix to be used in next step
    _rotation_total[_qp] = _rotation_increment[_qp] * _rotation_total_old[_qp];

    // Compute dstress_dstrain
    _Jacobian_mult[_qp] = elasticity_tensor_rotated; // This is NOT the exact jacobian
  }

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}
