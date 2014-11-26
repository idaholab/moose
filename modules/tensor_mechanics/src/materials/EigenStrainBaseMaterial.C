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
    DerivativeMaterialInterface<LinearElasticMaterial>(name, parameters),

    _c(coupledValue("c")),
    _c_name(getParam<VariableName>("c")),

    _eigenstrain_name(_base_name + "eigenstrain"),
    _eigenstrain(declareProperty<RankTwoTensor>(_eigenstrain_name)),
    _deigenstrain_dc(declarePropertyDerivative<RankTwoTensor>(_eigenstrain_name, _c_name)),
    _d2eigenstrain_dc2(declarePropertyDerivative<RankTwoTensor>(_eigenstrain_name, _c_name, _c_name)),

    _delasticity_tensor_dc(declarePropertyDerivative<ElasticityTensorR4>(_elasticity_tensor_name, _c_name)),
    _d2elasticity_tensor_dc2(declarePropertyDerivative<ElasticityTensorR4>(_elasticity_tensor_name, _c_name, _c_name))
{
}

RankTwoTensor EigenStrainBaseMaterial::computeStressFreeStrain()
{
  RankTwoTensor stress_free_strain = LinearElasticMaterial::computeStressFreeStrain();

  computeEigenStrain();

  stress_free_strain -= _eigenstrain[_qp];

  return stress_free_strain;
}
