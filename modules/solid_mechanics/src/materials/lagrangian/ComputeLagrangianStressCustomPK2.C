//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressCustomPK2.h"

registerMooseObject("SolidMechanicsApp", ComputeLagrangianStressCustomPK2);

InputParameters
ComputeLagrangianStressCustomPK2::validParams()
{
  InputParameters params = ComputeLagrangianStressPK1::validParams();
  params.addRequiredParam<MaterialPropertyName>("custom_pk2_stress", "The name of the PK2 stress.");
  params.addRequiredParam<MaterialPropertyName>(
      "custom_pk2_jacobian", "The name of the PK2 Jacobian (w.r.t. the deformation gradient).");
  return params;
}

ComputeLagrangianStressCustomPK2::ComputeLagrangianStressCustomPK2(
    const InputParameters & parameters)
  : ComputeLagrangianStressPK1(parameters),
    _pk2(getMaterialProperty<RankTwoTensor>("custom_pk2_stress")),
    _dpk2_dF(getMaterialProperty<RankFourTensor>("custom_pk2_jacobian"))
{
  if (!_large_kinematics)
    paramError("large_kinematics", "This material requires large kinematics to be enabled.");
}

void
ComputeLagrangianStressCustomPK2::computeQpPK1Stress()
{
  _pk1_stress[_qp] = _F[_qp] * _pk2[_qp];

  usingTensorIndices(i, j, k, l, m);
  _pk1_jacobian[_qp] = _F[_qp].times<i, m, m, j, k, l>(_dpk2_dF[_qp]);
}
