//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeLinearElasticStress.h"

registerMooseObject("TensorMechanicsApp", ADComputeLinearElasticStress);

InputParameters
ADComputeLinearElasticStress::validParams()
{
  InputParameters params = ADComputeStressBase::validParams();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

ADComputeLinearElasticStress::ADComputeLinearElasticStress(const InputParameters & parameters)
  : ADComputeStressBase(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getADMaterialProperty<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ADComputeLinearElasticStress::initialSetup()
{
  if (this->template hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains; use "
               "ADComputeFiniteStrainElasticStress for simulations using incremental and finite "
               "strains.");
}

void
ADComputeLinearElasticStress::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];
}
