//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseStressMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", TwoPhaseStressMaterial);

InputParameters
TwoPhaseStressMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a global stress in a two phase model");
  params.addParam<MaterialPropertyName>(
      "h", "h", "Switching Function Material that provides h(eta)");
  params.addRequiredParam<std::string>("base_A", "Base name for the Phase A strain.");
  params.addRequiredParam<std::string>("base_B", "Base name for the Phase B strain.");
  params.addParam<std::string>("base_name", "Base name for the computed global stress (optional).");
  return params;
}

TwoPhaseStressMaterial::TwoPhaseStressMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_eta(getMaterialProperty<Real>("h")),

    _base_A(getParam<std::string>("base_A") + "_"),
    _stress_A(getMaterialPropertyByName<RankTwoTensor>(_base_A + "stress")),
    _dstress_dstrain_A(getMaterialPropertyByName<RankFourTensor>(_base_A + "Jacobian_mult")),

    _base_B(getParam<std::string>("base_B") + "_"),
    _stress_B(getMaterialPropertyByName<RankTwoTensor>(_base_B + "stress")),
    _dstress_dstrain_B(getMaterialPropertyByName<RankFourTensor>(_base_B + "Jacobian_mult")),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _dstress_dstrain(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _global_extra_stress(getDefaultMaterialProperty<RankTwoTensor>("extra_stress"))
{
}

void
TwoPhaseStressMaterial::computeQpProperties()
{
  _stress[_qp] = _h_eta[_qp] * _stress_B[_qp] + (1.0 - _h_eta[_qp]) * _stress_A[_qp];

  // Add in global extra stress
  _stress[_qp] += _global_extra_stress[_qp];

  _dstress_dstrain[_qp] =
      _h_eta[_qp] * _dstress_dstrain_B[_qp] + (1.0 - _h_eta[_qp]) * _dstress_dstrain_A[_qp];
}
