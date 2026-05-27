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
  // PK1 = F_ust * PK2(F_stab) -- F-bar enters only through PK2 via the strain calc's
  // F-bar'd `_F` (which is what NEML2 sees as `forces/F`). The wrap uses F_ust so the
  // residual matches the new convention.
  _pk1_stress[_qp] = _F_ust[_qp] * _pk2[_qp];

  // dPK1/d(F_ust) = del_ik PK2_lj  +  F_ust * dPK2/d(F_stab) * d(F_stab)/d(F_ust).
  usingTensorIndices(i_, j_, k_, l_);
  const auto I = RankTwoTensor::Identity();
  const RankFourTensor termA = I.template times<i_, k_, j_, l_>(_pk2[_qp].transpose());
  // Default path: full F-bar local correction. Equals `_dpk2_dF` when F-bar is off (since
  // `_d_F_stab_d_F_ust == IdentityFour` then).
  const RankFourTensor dPK2_dFust = _dpk2_dF[_qp] * _d_F_stab_d_F_ust[_qp];
  _pk1_jacobian[_qp] = termA + dPK2_dFust.singleProductI(_F_ust[_qp]);
  // Bypass-F-bar variant for specialty kernels (WPS, homogenization).
  _pk1_jacobian_bypass_fbar[_qp] = termA + _dpk2_dF[_qp].singleProductI(_F_ust[_qp]);
}

void
ComputeLagrangianStressCustomPK2::computeQpCauchyStress()
{
  // sigma = (1/J_ust) F_ust * PK2(F_stab) * F_ust^T. Computed directly because the PK1
  // base's sigma chain assumes `pk1_jacobian = constitutive dPK1/d(F_stab)`, but here PK1
  // depends on F_ust directly (Term A above) -- already folded into _pk1_jacobian.
  const Real J_ust = _F_ust[_qp].det();
  _cauchy_stress[_qp] = _F_ust[_qp] * _pk2[_qp] * _F_ust[_qp].transpose() / J_ust;

  // cauchy_jacobian = dsigma/d(dL). sigma depends on dL only through PK2(F_stab(dL))
  // (F_ust is constant w.r.t. dL since dL is computed from F_stab via the kinematic helper).
  //   dPK2/d(dL) = dPK2/d(F_stab) * d(F_stab)/d(dL) = _dpk2_dF *
  //   (_d_spatial_velocity_increment_d_F)^{-1}
  const RankFourTensor dPK2_d_dL = _dpk2_dF[_qp] * _d_spatial_velocity_increment_d_F[_qp].inverse();
  _cauchy_jacobian[_qp] = dPK2_d_dL.singleProductI(_F_ust[_qp]).singleProductJ(_F_ust[_qp]) / J_ust;
}
