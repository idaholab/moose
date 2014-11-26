#include "EigenStrainBaseMaterial.h"

template<>
InputParameters validParams<EigenStrainBaseMaterial>()
{
  InputParameters params = validParams<LinearElasticMaterial>();
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

EigenStrainBaseMaterial::EigenStrainBaseMaterial(const std::string & name,
                                                 InputParameters parameters) :
    LinearElasticMaterial(name, parameters),
    _eigenstrain(declareProperty<RankTwoTensor>("eigenstrain")),
    _deigenstrain_dc(declareProperty<RankTwoTensor>("deigenstrain_dc")),
    _d2eigenstrain_dc2(declareProperty<RankTwoTensor>("d2eigenstrain_dc2")),
    _delasticity_tensor_dc(declareProperty<ElasticityTensorR4>("delasticity_tensor_dc")),
    _d2elasticity_tensor_dc2(declareProperty<ElasticityTensorR4>("d2elasticity_tensor_dc2")),
    _c(coupledValue("c"))
{
}

RankTwoTensor EigenStrainBaseMaterial::computeStressFreeStrain()
{
  RankTwoTensor stress_free_strain = LinearElasticMaterial::computeStressFreeStrain();

  computeEigenStrain();

  stress_free_strain -= _eigenstrain[_qp];

  return stress_free_strain;
}
