/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(declarePropertyOld<RankTwoTensor>(_base_name + "stress"))
{
}

void
ComputeFiniteStrainElasticStress::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  RankTwoTensor intermediate_stress =
      _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; // This is NOT the exact jacobian
}
