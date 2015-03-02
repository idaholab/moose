#include "ComputeElasticityStress.h"

template<>
InputParameters validParams<ComputeElasticityStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for small or finite strains");
  return params;
}

ComputeElasticityStress::ComputeElasticityStress(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<ComputeStressBase>(name, parameters),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _is_finite_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _strain_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(_is_finite_strain ? &declarePropertyOld<RankTwoTensor>(_base_name + "stress") : NULL)
{
}

void
ComputeElasticityStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  if (_is_finite_strain)
    (*_stress_old)[_qp] = _stress[_qp];
}

void
ComputeElasticityStress::computeQpStress()
{
  if (_is_finite_strain)
  {
    // stress = s_old + C * de
    RankTwoTensor intermediate_stress = (*_stress_old)[_qp] + _elasticity_tensor[_qp]*_strain_increment[_qp]; //Calculate stress in intermediate configruation

    //Rotate the stress to the current configuration
    _stress[_qp] = _rotation_increment[_qp]*intermediate_stress*_rotation_increment[_qp].transpose();

    //Assign value for elastic strain, which is equal to the total strain
    _elastic_strain[_qp] = _total_strain[_qp];

    //Compute dstress_dstrain
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; //This is NOT the exact jacobian
  }
  else
  {
    // stress = C * e
    _stress[_qp] = _elasticity_tensor[_qp]*_total_strain[_qp];

    //Assign value for elastic strain, which is equal to the total strain
    _elastic_strain[_qp] = _total_strain[_qp];

    //Compute dstress_dstrain
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  }
}
