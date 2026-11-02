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
  // Wrap PK1 -> sigma using the *unstabilized* F. F-bar enters only through the constitutive
  // PK1's strain dependence (via the F-bar'd `_f_inv`); the geometric wrap uses F_ust so
  // the residual matches OLD.
  //
  // All RankFourTensor algebra below builds Jacobian-only quantities
  // (`_cauchy_jacobian`, the post-multiplied `_pk1_jacobian`, and `_pk1_jacobian_bypass_fbar`).
  // Skip them on residual-only sweeps.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();
  if (_large_kinematics)
  {
    const Real J_ust = _F_ust[_qp].det();
    _cauchy_stress[_qp] = _pk1_stress[_qp] * _F_ust[_qp].transpose() / J_ust;

    if (!need_jacobian)
      return;

    // sigma = (1/J(F_ust)) * PK1(F_stab) * F_ust^T. cauchy_jacobian = dsigma/d(dL) at fixed F_ust;
    // the kernels supply d(dL) including the F-bar terms.
    if (needCauchyJacobian())
      _cauchy_jacobian[_qp] = _pk1_jacobian[_qp].singleProductJ(_F_ust[_qp]) / J_ust *
                              _d_deformation_gradient_increment_d_F[_qp].inverse();

    // dsigma/d(F_ust) at fixed PK1: (1/J) * PK1 * d(F_ust^T)/d(F_ust) - sigma (x) F_ust^{-T}
    if (isPropertyActive(_d_cauchy_d_F_ust.id()))
    {
      usingTensorIndices(i_, j_, m_, n_);
      const auto I = RankTwoTensor::Identity();
      _d_cauchy_d_F_ust[_qp] = _pk1_stress[_qp].times<i_, n_, j_, m_>(I) / J_ust -
                               _cauchy_stress[_qp].times<i_, j_, n_, m_>(_F_ust[_qp].inverse());
    }

    // Chain _pk1_jacobian to F_ust and publish the variant without the local F-bar factor
    const RankFourTensor pk1_jac_no_fbar = _pk1_jacobian[_qp];
    _pk1_jacobian[_qp] = _pk1_jacobian[_qp] * _d_F_stab_d_F_ust[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = pk1_jac_no_fbar;
  }
  // Small deformations: sigma = PK1 (no wrap). For TL, the kernel chain wants
  // dPK1/d(F_ust) -- pk1_jacobian (constitutive's dPK1/d(F_stab)) must be chained to
  // F_ust via `_d_F_stab_d_F_ust`. For UL, cauchy_jacobian = dsigma/d(dL) = pk1_jacobian
  // (since the constitutive's dPK1/d(F_stab) is essentially dsigma/d(dL) at small kin).
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    if (!need_jacobian)
      return;
    if (needCauchyJacobian())
      _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = _pk1_jacobian[_qp];
    _pk1_jacobian[_qp] = _pk1_jacobian[_qp] * _d_F_stab_d_F_ust[_qp];
  }
}

RankFourTensor
ComputeLagrangianStressPK1::pk2WrapDerivative(const RankTwoTensor & S) const
{
  // dsigma_ij/dF_mn = (delta_im (S F^T)_nj + (F S)_in delta_jm) / det(F) - sigma_ij F^-1_nm
  usingTensorIndices(i_, j_, m_, n_);
  const auto I = RankTwoTensor::Identity();
  const RankTwoTensor & F = _F_ust[_qp];
  return (I.times<i_, m_, n_, j_>(S * F.transpose()) + (F * S).times<i_, n_, j_, m_>(I)) / F.det() -
         _cauchy_stress[_qp].times<i_, j_, n_, m_>(F.inverse());
}
