//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSteinbachApelStrain.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ComputeSteinbachApelStrain);

InputParameters
ComputeSteinbachApelStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name", "Base name for the computed strain.");
  params.addParam<MaterialPropertyName>(
      "h", "h", "Switching Function Material that provides h(eta)");
  params.addRequiredParam<std::string>("base_A", "Base name for the alpha phase.");
  params.addRequiredParam<std::string>("base_B", "Base name for the beta phase.");
  return params;
}

ComputeSteinbachApelStrain::ComputeSteinbachApelStrain(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_eta(getMaterialProperty<Real>("h")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _base_A(getParam<std::string>("base_A") + "_"),
    _base_B(getParam<std::string>("base_B") + "_"),
    _elasticity_tensor_B_name(_base_B + "elasticity_tensor"),
    _elasticity_tensor_B(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_B_name)),
    _compliance_tensor_A_name(_base_A + "compliance_tensor"),
    _compliance_tensor_A(getMaterialPropertyByName<RankFourTensor>(_compliance_tensor_A_name)),
    _global_strain_name("mechanical_strain"),
    _global_mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_global_strain_name)),
    _ref(declareProperty<RankFourTensor>("ref")),
    _ref_inv(declareProperty<RankFourTensor>("ref_inv")),
    _mechanical_strain_b(declareProperty<RankTwoTensor>(_base_B + "mechanical_strain")),
    _mechanical_strain_a(declareProperty<RankTwoTensor>(_base_A + "mechanical_strain")),
    _identity(declareProperty<RankFourTensor>("identity"))
{
}

void
ComputeSteinbachApelStrain::computeQpProperties()
{
  for (unsigned int i = 0; i < 6; i++)
    _identity_two[i][i] = 1.0;

  _identity[_qp](0, 0, 0, 0) = _identity_two[0][0];
  _identity[_qp](1, 1, 1, 1) = _identity_two[1][1];
  _identity[_qp](2, 2, 2, 2) = _identity_two[2][2];
  _identity[_qp](1, 2, 1, 2) = _identity_two[3][3];
  _identity[_qp](0, 2, 0, 2) = _identity_two[4][4];
  _identity[_qp](0, 1, 0, 1) = _identity_two[5][5];

  _ref[_qp] = _h_eta[_qp] * _identity[_qp] + 
              (1.0 - _h_eta[_qp]) * _compliance_tensor_A[_qp] * _elasticity_tensor_B[_qp];

  _ref_inv[_qp] = _ref[_qp].invSymm();

  _mechanical_strain_b[_qp] = _ref_inv[_qp] * _global_mechanical_strain[_qp];
  _mechanical_strain_a[_qp] = 
      _compliance_tensor_A[_qp] * _elasticity_tensor_B[_qp] * _mechanical_strain_b[_qp];
}
