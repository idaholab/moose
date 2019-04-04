//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeLinearElasticStress.h"

registerADMooseObject("TensorMechanicsApp", ADComputeLinearElasticStress);

defineADValidParams(
    ADComputeLinearElasticStress,
    ADComputeStressBase,
    params.addClassDescription("Compute stress using elasticity for small strains"););

template <ComputeStage compute_stage>
ADComputeLinearElasticStress<compute_stage>::ADComputeLinearElasticStress(
    const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(adGetADMaterialProperty<RankFourTensor>(_elasticity_tensor_name))
{
}

template <ComputeStage compute_stage>
void
ADComputeLinearElasticStress<compute_stage>::initialSetup()
{
  if (this->template hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains; use "
               "ADComputeFiniteStrainElasticStress for simulations using incremental and finite "
               "strains.");
}

template <ComputeStage compute_stage>
void
ADComputeLinearElasticStress<compute_stage>::computeQpStress()
{
  // stress(PK1) = F * PK2 = F * C * e
  _stress[_qp] = _deformation_gradient[_qp] * (_elasticity_tensor[_qp] * _mechanical_strain[_qp]);

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}
