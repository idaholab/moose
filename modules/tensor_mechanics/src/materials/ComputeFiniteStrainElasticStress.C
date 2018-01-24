//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFiniteStrainElasticStress.h"

template <>
InputParameters
validParams<ComputeFiniteStrainElasticStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for finite strains");
  return params;
}

ComputeFiniteStrainElasticStress::ComputeFiniteStrainElasticStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    GuaranteeConsumer(this),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

void
ComputeFiniteStrainElasticStress::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ComputeFiniteStrainElasticStress can only be used with elasticity tensor materials "
               "that guarantee isotropic tensors.");
}

void
ComputeFiniteStrainElasticStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
}

void
ComputeFiniteStrainElasticStress::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  RankTwoTensor intermediate_stress;

  intermediate_stress =
      _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + _strain_increment[_qp]);

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // InitialStress Deprecation: remove the following 2 lines
  rotateQpInitialStress();
  addQpInitialStress();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; // This is NOT the exact jacobian
}

void
ComputeFiniteStrainElasticStress::rotateQpInitialStress()
{
  if (_initial_stress_provided)
    (*_initial_stress)[_qp] = _rotation_increment[_qp] * (*_initial_stress_old)[_qp] *
                              _rotation_increment[_qp].transpose();
}
