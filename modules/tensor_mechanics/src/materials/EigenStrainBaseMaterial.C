/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EigenStrainBaseMaterial.h"

template <>
InputParameters
validParams<EigenStrainBaseMaterial>()
{
  InputParameters params = validParams<LinearElasticMaterial>();
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

EigenStrainBaseMaterial::EigenStrainBaseMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<LinearElasticMaterial>(parameters),

    _c(coupledValue("c")),
    _c_name(getVar("c", 0)->name()),

    _eigenstrain_name(_base_name + "eigenstrain"),
    _eigenstrain(declareProperty<RankTwoTensor>(_eigenstrain_name)),

    // the derivatives of elastic strain w.r.t c are provided here
    _delastic_strain_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _c_name)),
    _d2elastic_strain_dc2(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _c_name, _c_name)),

    _delasticity_tensor_dc(
        declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, _c_name)),
    _d2elasticity_tensor_dc2(
        declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, _c_name, _c_name))
{
  mooseDeprecated(
      "EigenStrainBaseMaterial is deprecated.   Please use ComputeVariableEigenstrain instead.");
}

RankTwoTensor
EigenStrainBaseMaterial::computeStressFreeStrain()
{
  RankTwoTensor stress_free_strain = LinearElasticMaterial::computeStressFreeStrain();

  computeEigenStrain();

  stress_free_strain -= _eigenstrain[_qp];

  return stress_free_strain;
}
