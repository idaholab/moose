/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeRadialReturnMappingStress.h"
#include "DiscreteMaterial.h"

template<>
InputParameters validParams<ComputeRadialReturnMappingStress>()
{
  InputParameters params = validParams<ComputeFiniteStrainElasticStress>();
  params.addClassDescription("Compute stress using a radial return mapping implementation for creep or creep combined with plasticity");
  params.addRequiredParam<DiscreteMaterialName>("discrete_radial_material", "The material object to recompute.");
  return params;
}

ComputeRadialReturnMappingStress::ComputeRadialReturnMappingStress(const InputParameters & parameters) :
    ComputeFiniteStrainElasticStress(parameters),
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),
    _radial_return_stress(getMaterialProperty<RankTwoTensor>("radial_return_stress")),
    _inelastic_strain_increment(getMaterialProperty<RankTwoTensor>("inelastic_strain_increment")),
    _discrete_radial_return_material(getDiscreteMaterial("discrete_radial_material"))
{
}

void
ComputeRadialReturnMappingStress::initQpStatefulProperties()
{
  ComputeFiniteStrainElasticStress::initQpStatefulProperties();

  _elastic_strain_old[_qp] = _elastic_strain[_qp];
}

void
ComputeRadialReturnMappingStress::computeQpStress()
{
  // compute the stress increment required to return to the yield surface radially (J2)
  _discrete_radial_return_material.computeProperties(_qp);

  //Rotate the stress to the current configuration after updating to include the radial return value
  _stress[_qp] = _rotation_increment[_qp] * ( _radial_return_stress[_qp] + _stress_old[_qp] ) * _rotation_increment[_qp].transpose();

  // Update the elastic strain in the intermediate configuration,
  // where the inelastic strain is accumulated in mechanical strain (via _strain_increment) but not in elastic strain
  _elastic_strain[_qp] = _strain_increment[_qp] - _inelastic_strain_increment[_qp] + _elastic_strain_old[_qp];

  //Rotate elastic strain to current configuration
  _elastic_strain[_qp] = _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();

  //Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; //This is NOT the exact jacobian
}
