//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearViscoelasticStress.h"

registerMooseObject("TensorMechanicsApp", ComputeLinearViscoelasticStress);

InputParameters
ComputeLinearViscoelasticStress::validParams()
{
  InputParameters params = ComputeLinearElasticStress::validParams();
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
      "elasticity_tensor_inv",
      "elasticity_tensor_inv",
      "name of the real compliance tensor (defined by a LinearViscoelasticityBase material)");
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
    _elasticity_tensor_inv(getMaterialProperty<RankFourTensor>("elasticity_tensor_inv"))
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
      _mechanical_strain[_qp] - (_apparent_elasticity_tensor[_qp] * _elasticity_tensor_inv[_qp]) *
                                    (_mechanical_strain[_qp] - _apparent_creep_strain[_qp]);

  _elastic_strain[_qp] = _mechanical_strain[_qp] - _creep_strain[_qp];

  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
