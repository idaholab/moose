//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressPK2.h"

InputParameters
ComputeLagrangianStressPK2::validParams()
{
  InputParameters params = ComputeLagrangianStressPK1::validParams();
  return params;
}

ComputeLagrangianStressPK2::ComputeLagrangianStressPK2(const InputParameters & parameters)
  : ComputeLagrangianStressPK1(parameters),
    _E(declareProperty<RankTwoTensor>(_base_name + "green_lagrange_strain")),
    _S(declareProperty<RankTwoTensor>(_base_name + "pk2_stress")),
    _C(declareProperty<RankFourTensor>(_base_name + "pk2_jacobian"))
{
}

void
ComputeLagrangianStressPK2::computeQpCauchyStress()
{
  // PK2 has its own sigma-wrap structure: sigma = (1/J_ust) F_ust * S * F_ust^T. The PK1 base's
  // sigma-chain (which assumes pk1_jacobian = constitutive dPK1/dF_stab + _d_F_stab_d_F_ust)
  // doesn't fit here, so compute sigma and cauchy_jacobian directly from S and dS/dE.
  //
  // The R4 chain for `_cauchy_jacobian` is Jacobian-only; the wrap to `_cauchy_stress` is
  // not. Gate accordingly.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();
  if (_large_kinematics)
  {
    const Real J_ust = _F_ust[_qp].det();
    _cauchy_stress[_qp] = _F_ust[_qp] * _S[_qp] * _F_ust[_qp].transpose() / J_ust;

    if (!need_jacobian)
      return;

    // cauchy_jacobian = dsigma/d(dL). sigma depends on dL only through S(E(F_stab(dL))); the
    // F_ust factors in the wrap are CONSTANT w.r.t. dL (F_ust does not depend on dL --
    // dL is computed from F_stab via the kinematic helper, and F-bar relates F_stab to
    // F_ust). So:
    //   dsigma/d(dL) = (1/J_ust) F_ust * dS/d(dL) * F_ust^T
    //            = (1/J_ust) F_ust * _C * dE/d(F_stab) * inverse(d(dL)/d(F_stab)) * F_ust^T.
    usingTensorIndices(i_, j_, k_, l_);
    const auto I2 = RankTwoTensor::Identity();
    const RankFourTensor dE_dFstab = 0.5 * (I2.template times<i_, l_, j_, k_>(_F[_qp].transpose()) +
                                            _F[_qp].transpose().template times<i_, k_, j_, l_>(I2));
    const RankFourTensor dE_d_dL = dE_dFstab * _d_deformation_gradient_increment_d_F[_qp].inverse();
    const RankFourTensor dS_d_dL = _C[_qp] * dE_d_dL;
    _cauchy_jacobian[_qp] = dS_d_dL.singleProductI(_F_ust[_qp]).singleProductJ(_F_ust[_qp]) / J_ust;
  }
  else
  {
    _cauchy_stress[_qp] = _S[_qp];
    if (need_jacobian)
      _cauchy_jacobian[_qp] = _C[_qp];
  }
}

void
ComputeLagrangianStressPK2::computeQpPK1Stress()
{
  // Green-Lagrange strain uses the F-bar-stabilized F (`_F` from the strain calc) so
  // the constitutive law receives the stabilized strain -- F-bar's purpose is precisely
  // to feed a volumetrically-corrected strain into the constitutive update.
  _E[_qp] = 0.5 * (_F[_qp].transpose() * _F[_qp] - RankTwoTensor::Identity());

  // PK2 update (constitutive). This populates `_S` (always needed) and `_C` (= dPK2/dE,
  // Jacobian-only). PK2-subclass implementers that want to skip `_C` on residual sweeps
  // can check `_fe_problem.currentlyComputingJacobian()` themselves; the wrap below
  // doesn't read `_C` in the residual path.
  computeQpPK2Stress();

  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();

  // PK2 -> PK1 wrap uses the *unstabilized* F so the residual matches OLD. The constitutive
  // PK2 still carries the F-bar effect via its dependence on E (Green-Lagrange of F_stab).
  if (_large_kinematics)
  {
    _pk1_stress[_qp] = _F_ust[_qp] * _S[_qp];
    if (!need_jacobian)
      return;

    usingTensorIndices(i_, j_, k_, l_);
    // dE/d(F_stab) (E is computed from F_stab).
    RankFourTensor dE_dFstab =
        0.5 * (RankTwoTensor::Identity().times<i_, l_, j_, k_>(_F[_qp].transpose()) +
               _F[_qp].transpose().times<i_, k_, j_, l_>(RankTwoTensor::Identity()));
    // Chain dE/d(F_ust) = dE/d(F_stab) * d(F_stab)/d(F_ust). With F-bar off
    // _d_F_stab_d_F_ust = IdentityFour and this collapses to dE_dFstab.
    const RankFourTensor dE_dFust = dE_dFstab * _d_F_stab_d_F_ust[_qp];

    // dPK1/d(F_ust) = d(F_ust)/d(F_ust) * S + F_ust * dS/d(F_ust)
    //   d(F_ust)/d(F_ust)*S gives I x S^T (per the existing template, with S^T because
    //   PK1 = F*S and we differentiate F).
    //   F_ust * dS/d(F_ust) via dS/dE * dE/d(F_ust).
    const RankFourTensor termA =
        RankTwoTensor::Identity().times<i_, k_, j_, l_>(_S[_qp].transpose());
    _pk1_jacobian[_qp] = termA + (_C[_qp] * dE_dFust).singleProductI(_F_ust[_qp]);
    // Bypass-F-bar variant: same Term A; sigma chain uses dE_dFstab (without
    // _d_F_stab_d_F_ust). Used by specialty kernels whose coupled variable bypasses
    // F-bar.
    _pk1_jacobian_bypass_fbar[_qp] = termA + (_C[_qp] * dE_dFstab).singleProductI(_F_ust[_qp]);
  }
  // Small deformations: PK1 = PK2 = sigma; PK2 chain to PK1 still needs the F-bar local
  // contribution through dE/d(F_stab) * d(F_stab)/d(F_ust). For specialty bypass paths,
  // skip the F-bar factor.
  else
  {
    _pk1_stress[_qp] = _S[_qp];
    if (!need_jacobian)
      return;
    _pk1_jacobian[_qp] = _C[_qp] * _d_F_stab_d_F_ust[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = _C[_qp];
  }
}
