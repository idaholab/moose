/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableElasticConstantStress.h"

template <>
InputParameters
validParams<ComputeVariableElasticConstantStress>()
{
  InputParameters params = validParams<ComputeFiniteStrainElasticStress>();
  params.addClassDescription("Compute elastic stress for finite strains when the elasticity tensor "
                             "components change, e.g. the elastic constants are a function of "
                             "temperature");
  return params;
}

ComputeVariableElasticConstantStress::ComputeVariableElasticConstantStress(
    const InputParameters & parameters)
  : ComputeFiniteStrainElasticStress(parameters),
    _elasticity_tensor_old(getMaterialPropertyOld<RankFourTensor>(_base_name + "elasticity_tensor"))
{
}

void
ComputeVariableElasticConstantStress::computeQpStress()
{
  RankTwoTensor intermediate_stress;

  // Check to see if the C1111 elastic constant has changed before inverting
  // Once we introduce aniostropy into these models this check will need to be expanded/ coded more
  // craftily
  if (!MooseUtils::relativeFuzzyEqual(_elasticity_tensor_old[_qp](0, 0, 0, 0),
                                      _elasticity_tensor[_qp](0, 0, 0, 0)) &&
      !MooseUtils::relativeFuzzyEqual(_elasticity_tensor_old[_qp](0, 0, 0, 0), 0.0))
  {
    // Recover the old strain state by multiplying the old stress by the inverse old elasticity
    // tensor
    RankFourTensor old_Cijkl_inverse = _elasticity_tensor_old[_qp].invSymm();
    RankTwoTensor intermediate_strain =
        old_Cijkl_inverse * _stress_old[_qp] + _strain_increment[_qp];
    intermediate_stress = _elasticity_tensor[_qp] * intermediate_strain;
  }
  else // Calculate the stress in the intermediate configuration from the old stress state
    intermediate_stress = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; // This is NOT the exact jacobian
}
