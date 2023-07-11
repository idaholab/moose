//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHarmonicElasticityTensor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeHarmonicElasticityTensor);

InputParameters
ComputeHarmonicElasticityTensor::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a stiffness tensor in a two phase model");
  params.addParam<MaterialPropertyName>(
      "h", "h", "Switching Function Material that provides h(eta)");
  params.addRequiredParam<std::string>("base_A", "Base name for the Phase A");
  params.addRequiredParam<std::string>("base_B", "Base name for the Phase B");
  params.addParam<std::string>("base_name", "Base name (optional).");
  return params;
}

ComputeHarmonicElasticityTensor::ComputeHarmonicElasticityTensor(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_eta(getMaterialProperty<Real>("h")),

    _base_A(getParam<std::string>("base_A") + "_"),
    _compliance_A(getMaterialPropertyByName<RankFourTensor>(_base_A + "compliance_tensor")),

    _base_B(getParam<std::string>("base_B") + "_"),
    _compliance_B(getMaterialPropertyByName<RankFourTensor>(_base_B + "compliance_tensor")),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _compliance(declareProperty<RankFourTensor>(_base_name + "compliance_tensor")),
    _elasticity_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(declareProperty<RankFourTensor>(_base_name + "elasticity_tensor"))
{
}

void
ComputeHarmonicElasticityTensor::computeQpProperties()
{
  _compliance[_qp] = _h_eta[_qp] * _compliance_B[_qp] + (1.0 - _h_eta[_qp]) * _compliance_A[_qp];

  _elasticity_tensor[_qp] = _compliance[_qp].invSymm();
}
