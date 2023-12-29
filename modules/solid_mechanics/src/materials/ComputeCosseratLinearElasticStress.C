//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCosseratLinearElasticStress.h"

registerMooseObject("TensorMechanicsApp", ComputeCosseratLinearElasticStress);

InputParameters
ComputeCosseratLinearElasticStress::validParams()
{
  InputParameters params = ComputeCosseratStressBase::validParams();
  params.addClassDescription(
      "Compute Cosserat stress and couple-stress elasticity for small strains");
  return params;
}

ComputeCosseratLinearElasticStress::ComputeCosseratLinearElasticStress(
    const InputParameters & parameters)
  : ComputeCosseratStressBase(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ComputeCosseratLinearElasticStress::initialSetup()
{
  if (hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains");
}

void
ComputeCosseratLinearElasticStress::computeQpStress()
{
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  _stress_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];

  _elastic_strain[_qp] = _mechanical_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  _Jacobian_mult_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp];
}
