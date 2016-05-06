/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeReturnMappingStress.h"

template<>
InputParameters validParams<ComputeReturnMappingStress>()
{
  InputParameters params = validParams<ComputeFiniteStrainElasticStress>();
  params.addClassDescription("Compute stress using a radial return mapping implementation for creep or creep combined with plasticity");
  params.addRequiredParam<MaterialName>("return_mapping_stress_model", "The material object to use to calculate the return stress increment in a self-contained recompute iteration.");
  return params;
}

ComputeReturnMappingStress::ComputeReturnMappingStress(const InputParameters & parameters) :
    ComputeFiniteStrainElasticStress(parameters),
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),
    _inelastic_strain_increment(getMaterialProperty<RankTwoTensor>("inelastic_strain_increment")),
    _recompute_return_material(getMaterial("return_mapping_stress_model"))
{
}

void
ComputeReturnMappingStress::initQpStatefulProperties()
{
  ComputeFiniteStrainElasticStress::initQpStatefulProperties();

  _elastic_strain_old[_qp] = _elastic_strain[_qp];
}

void
ComputeReturnMappingStress::computeQpStress()
{
  // compute the stress increment (as a tensor) and the inelastic strain increment (tensor) required to return to the yield surface
  _recompute_return_material.computePropertiesAtQp(_qp);

  // Update the elastic strain in the intermediate configuration,
  // where the inelastic strain is accumulated in mechanical strain (via _strain_increment) but not in elastic strain
  _elastic_strain[_qp] = _strain_increment[_qp] - _inelastic_strain_increment[_qp] + _elastic_strain_old[_qp];

  //Rotate elastic strain to current configuration
  _elastic_strain[_qp] = _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();

  // Compute the stress in the current configuration
  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  //Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; //This is NOT the exact jacobian
}
