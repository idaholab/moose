/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeLinearViscoelasticStress.h"

template <>
InputParameters
validParams<ComputeLinearViscoelasticStress>()
{
  InputParameters params = validParams<ComputeLinearElasticStress>();
  params.addClassDescription("Divides total strain into elastic + creep + eigenstrains");
  params.addParam<std::string>(
      "apparent_creep_strain",
      "apparent_creep_strain",
      "name of the apparent creep strain (defined by a LinearViscoelasticityBase material)");
  params.addParam<std::string>(
      "apparent_elasticity_tensor",
      "apparent_elasticity_tensor",
      "name of the apparent elasticity tensor (defined by a LinearViscoelasticityBase material)");
  params.addParam<std::string>(
      "instantaneous_elasticity_tensor_inv",
      "instantaneous_elasticity_tensor_inv",
      "name of the apparent compliance tensor (defined by a LinearViscoelasticityBase material)");
  return params;
}

ComputeLinearViscoelasticStress::ComputeLinearViscoelasticStress(const InputParameters & parameters)
  : ComputeLinearElasticStress(parameters),
    _creep_strain(declareProperty<RankTwoTensor>(
        isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(
        isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _apparent_creep_strain(getMaterialProperty<RankTwoTensor>("apparent_creep_strain")),
    _apparent_elasticity_tensor(getMaterialProperty<RankFourTensor>("apparent_elasticity_tensor")),
    _instantaneous_elasticity_tensor_inv(
        getMaterialProperty<RankFourTensor>("instantaneous_elasticity_tensor_inv"))
{
}

void
ComputeLinearViscoelasticStress::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
}

void
ComputeLinearViscoelasticStress::computeQpStress()
{
  _creep_strain[_qp] =
      _mechanical_strain[_qp] -
      (_apparent_elasticity_tensor[_qp] * _instantaneous_elasticity_tensor_inv[_qp]) *
          (_mechanical_strain[_qp] - _apparent_creep_strain[_qp]);

  _elastic_strain[_qp] = _mechanical_strain[_qp] - _creep_strain[_qp];

  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
