//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseStrainMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", TwoPhaseStrainMaterial);

InputParameters
TwoPhaseStrainMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a global strain in a two phase model");
  params.addParam<MaterialPropertyName>(
      "h", "h", "Switching Function Material that provides h(eta)");
  params.addRequiredParam<std::string>("base_A", "Base name for the Phase A strain.");
  params.addRequiredParam<std::string>("base_B", "Base name for the Phase B strain.");
  return params;
}

TwoPhaseStrainMaterial::TwoPhaseStrainMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_eta(getMaterialProperty<Real>("h")),

    _base_A(getParam<std::string>("base_A") + "_"),
    _mechanical_strain_A(getMaterialPropertyByName<RankTwoTensor>(_base_A + "mechanical_strain")),

    _base_B(getParam<std::string>("base_B") + "_"),
    _mechanical_strain_B(getMaterialPropertyByName<RankTwoTensor>(_base_B + "mechanical_strain")),

    _strain(declareProperty<RankTwoTensor>("mechanical_strain"))
{
}

void
TwoPhaseStrainMaterial::computeQpProperties()
{
  _strain[_qp] = 
      _h_eta[_qp] * _mechanical_strain_B[_qp] + (1.0 - _h_eta[_qp]) * _mechanical_strain_A[_qp];
}
