//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteStrainElasticStress.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

registerMooseObject("TensorMechanicsApp", ADComputeFiniteStrainElasticStress);
registerMooseObject("TensorMechanicsApp", ADSymmetricFiniteStrainElasticStress);

template <typename R2, typename R4>
InputParameters
ADComputeFiniteStrainElasticStressTempl<R2, R4>::validParams()
{
  InputParameters params = ADComputeStressBase::validParams();
  params.addClassDescription("Compute stress using elasticity for finite strains");
  return params;
}

template <typename R2, typename R4>
ADComputeFiniteStrainElasticStressTempl<R2, R4>::ADComputeFiniteStrainElasticStressTempl(
    const InputParameters & parameters)
  : ADComputeStressBaseTempl<R2>(parameters),
    GuaranteeConsumer(this),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(this->template getADMaterialProperty<R4>(_elasticity_tensor_name)),
    _strain_increment(
        this->template getADMaterialPropertyByName<R2>(_base_name + "strain_increment")),
    _rotation_total(this->template declareADProperty<RankTwoTensor>(_base_name + "rotation_total")),
    _rotation_total_old(
        this->template getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "rotation_total")),
    _rotation_increment(this->template getADMaterialPropertyByName<RankTwoTensor>(
        _base_name + "rotation_increment")),
    _stress_old(this->template getMaterialPropertyOldByName<R2>(_base_name + "stress")),
    _elastic_strain_old(
        this->template getMaterialPropertyOldByName<R2>(_base_name + "elastic_strain"))
{
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainElasticStressTempl<R2, R4>::initialSetup()
{
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainElasticStressTempl<R2, R4>::initQpStatefulProperties()
{
  ADComputeStressBaseTempl<R2>::initQpStatefulProperties();
  _rotation_total[_qp] = RankTwoTensor::Identity();
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainElasticStressTempl<R2, R4>::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  ADR2 intermediate_stress;

  if (hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    intermediate_stress =
        _elasticity_tensor[_qp] * (_strain_increment[_qp] + _elastic_strain_old[_qp]);
  else
  {
    // Rotate elasticity tensor to the intermediate configuration
    // That is, elasticity tensor is defined in the previous time step
    // This is consistent with the definition of strain increment
    // The stress is projected onto the current configuration a few lines below
    auto elasticity_tensor_rotated = _elasticity_tensor[_qp];
    elasticity_tensor_rotated.rotate(_rotation_total_old[_qp]);

    intermediate_stress =
        elasticity_tensor_rotated * (_elastic_strain_old[_qp] + _strain_increment[_qp]);

    // Update current total rotation matrix to be used in next step
    _rotation_total[_qp] = _rotation_increment[_qp] * _rotation_total_old[_qp];
  }
  // Rotate the stress state to the current configuration
  _stress[_qp] = intermediate_stress;
  _stress[_qp].rotate(_rotation_increment[_qp]);

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}

template class ADComputeFiniteStrainElasticStressTempl<RankTwoTensor, RankFourTensor>;
template class ADComputeFiniteStrainElasticStressTempl<SymmetricRankTwoTensor,
                                                       SymmetricRankFourTensor>;
