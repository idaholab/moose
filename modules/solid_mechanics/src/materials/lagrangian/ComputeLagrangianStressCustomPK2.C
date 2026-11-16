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
}

void
ComputeLagrangianStressCustomPK2::initialSetup()
{
  // Derives _large_kinematics from the strain calculator's LARGE_KINEMATICS guarantee.
  ComputeLagrangianStressBase::initialSetup();

  if (!_large_kinematics)
    mooseError("This material requires large kinematics; set large_kinematics = true on the "
               "ComputeLagrangianStrain calculator.");
}

void
ComputeLagrangianStressCustomPK2::computeQpPK1Stress()
{
  // Pbar = Fbar * PK2(Fbar), with Fbar the strain calc's F-bar'd `_F` (which is what NEML2 sees
  // as `forces/F`). The PK1 base pushes it forward with Fbar to the Cauchy stress.
  _pk1_stress[_qp] = _F[_qp] * _pk2[_qp];

  // The R4 algebra for `_pk1_jacobian` is read only on Jacobian sweeps; skip it on
  // residual-only sweeps.
  if (!_fe_problem.currentlyComputingJacobian() &&
      !_fe_problem.currentlyComputingResidualAndJacobian())
    return;

  // dPbar/dFbar = del_ik PK2_lj  +  Fbar * dPK2/dFbar.
  usingTensorIndices(i_, j_, k_, l_);
  const auto I = RankTwoTensor::Identity();
  _pk1_jacobian[_qp] = I.template times<i_, k_, j_, l_>(_pk2[_qp].transpose()) +
                       _dpk2_dF[_qp].singleProductI(_F[_qp]);
}
