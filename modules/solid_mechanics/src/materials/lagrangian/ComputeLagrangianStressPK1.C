//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressPK1.h"

InputParameters
ComputeLagrangianStressPK1::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();

  params.addClassDescription("Stress update based on the first Piola-Kirchhoff stress");

  return params;
}

ComputeLagrangianStressPK1::ComputeLagrangianStressPK1(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                     "inverse_incremental_deformation_gradient")),
    _inv_def_grad(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient"))
{
}

void
ComputeLagrangianStressPK1::computeQpStressUpdate()
{
  computeQpPK1Stress();
  computeQpCauchyStress(); // This could be "switched"
}

void
ComputeLagrangianStressPK1::computeQpCauchyStress()
{
  // Wrap PK1 → σ using the *unstabilized* F. F-bar enters only through the constitutive
  // PK1's strain dependence (via the F-bar'd `_f_inv`); the geometric wrap uses F_ust so
  // the residual matches OLD.
  if (_large_kinematics)
  {
    const RankTwoTensor F_ust_inv = _F_ust[_qp].inverse();
    const RankTwoTensor F_ust_invT = F_ust_inv.transpose();
    const Real J_ust = _F_ust[_qp].det();
    _cauchy_stress[_qp] = _pk1_stress[_qp] * _F_ust[_qp].transpose() / J_ust;

    usingTensorIndices(i_, j_, m_, n_);
    const auto I = RankTwoTensor::Identity();

    // dσ/d(F_ust) via the constitutive PK1's F_stab chain plus the geometric F_ust pieces.
    //   σ = (1/J(F_ust)) · PK1(F_stab) · F_ust^T
    //   dσ/d(F_ust) = (1/J) · dPK1/d(F_stab) · d(F_stab)/d(F_ust) · F_ust^T
    //               + (1/J) · PK1 · d(F_ust^T)/d(F_ust)
    //               − σ ⊗ F_ust^{-T}.
    // The middle factor `_d_F_stab_d_F_ust` is IdentityFour when F-bar is off, so the
    // first line collapses to the original PK1-jacobian chain.
    // Common geometric pieces (independent of F-bar):
    const RankFourTensor dsigma_dF_geom =
        _pk1_stress[_qp].times<i_, n_, j_, m_>(I) / J_ust -
        _cauchy_stress[_qp].times<i_, j_, n_, m_>(F_ust_inv);
    // dsigma_dF with the constitutive PK1 jacobian chained via F-bar (default path).
    RankFourTensor dsigma_dF =
        (_pk1_jacobian[_qp] * _d_F_stab_d_F_ust[_qp]).singleProductJ(_F_ust[_qp]) / J_ust;
    dsigma_dF += dsigma_dF_geom;
    _cauchy_jacobian[_qp] = dsigma_dF * _d_spatial_velocity_increment_d_F[_qp].inverse();

    // Bypass-F-bar variant: σ chain without the `_d_F_stab_d_F_ust` factor. Same
    // geometric pieces; constitutive PK1 jacobian used directly.
    RankFourTensor dsigma_dF_no_fbar =
        _pk1_jacobian[_qp].singleProductJ(_F_ust[_qp]) / J_ust;
    dsigma_dF_no_fbar += dsigma_dF_geom;

    // Update _pk1_jacobian and publish the bypass variant.
    const RankFourTensor pk1_jac_no_fbar = _pk1_jacobian[_qp];
    _pk1_jacobian[_qp] = _pk1_jacobian[_qp] * _d_F_stab_d_F_ust[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = pk1_jac_no_fbar;
  }
  // Small deformations: σ = PK1 (no wrap). For TL, the kernel chain wants
  // dPK1/d(F_ust) — pk1_jacobian (constitutive's dPK1/d(F_stab)) must be chained to
  // F_ust via `_d_F_stab_d_F_ust`. For UL, cauchy_jacobian = dσ/d(dL) = pk1_jacobian
  // (since the constitutive's dPK1/d(F_stab) is essentially dσ/d(dL) at small kin).
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = _pk1_jacobian[_qp];
    _pk1_jacobian[_qp] = _pk1_jacobian[_qp] * _d_F_stab_d_F_ust[_qp];
  }
}
