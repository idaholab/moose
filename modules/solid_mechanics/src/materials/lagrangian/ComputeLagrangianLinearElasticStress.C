//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianLinearElasticStress.h"

registerMooseObject("SolidMechanicsApp", ComputeLagrangianLinearElasticStress);

InputParameters
ComputeLagrangianLinearElasticStress::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

ComputeLagrangianLinearElasticStress::ComputeLagrangianLinearElasticStress(
    const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>(
        _base_name + getParam<MaterialPropertyName>("elasticity_tensor")))
{
}

void
ComputeLagrangianLinearElasticStress::computeQpSmallStress()
{
  _small_stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  _small_jacobian[_qp] = _elasticity_tensor[_qp];
}
