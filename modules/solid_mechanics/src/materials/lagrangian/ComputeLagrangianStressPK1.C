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
    _stabilized_deformation_gradient(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient"))
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
  // computeQpPK1Stress provides the constitutive PK1 stress Pbar and dPbar/dFbar at the
  // F-bar-stabilized deformation gradient Fbar = F_stab. F-bar replaces F only in the
  // constitutive model (de Souza Neto et al. 1996), so the Cauchy stress is the push-forward
  // with Fbar,
  //   sigma = Pbar Fbar^T / det(Fbar),
  // and the residual pulls it back with the unstabilized F, P = det(F) sigma F^{-T}.
  //
  // All RankFourTensor algebra below builds Jacobian-only quantities
  // (`_cauchy_jacobian`, `_pk1_jacobian`, and `_pk1_jacobian_bypass_fbar`).
  // Skip them on residual-only sweeps.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();
  if (_large_kinematics)
  {
    const RankTwoTensor & F_bar = _stabilized_deformation_gradient[_qp];
    const Real J_bar = F_bar.det();
    const RankTwoTensor P_bar = _pk1_stress[_qp];
    _cauchy_stress[_qp] = P_bar * F_bar.transpose() / J_bar;
    pullBackCauchyStress();

    if (!need_jacobian)
      return;

    // dsigma/dFbar = [dPbar/dFbar Fbar^T + Pbar d(Fbar^T)/dFbar] / det(Fbar) - sigma (x) Fbar^{-T}
    usingTensorIndices(i_, j_, m_, n_);
    const auto I = RankTwoTensor::Identity();
    const RankFourTensor dsigma_dF_bar =
        _pk1_jacobian[_qp].singleProductJ(F_bar) / J_bar + P_bar.times<i_, n_, j_, m_>(I) / J_bar -
        _cauchy_stress[_qp].times<i_, j_, n_, m_>(_inv_def_grad[_qp]);
    _cauchy_jacobian[_qp] = dsigma_dF_bar * _d_deformation_gradient_increment_d_F[_qp].inverse();
    pullBackCauchyJacobian(dsigma_dF_bar);
  }
  // Small deformations: sigma = PK1 (no wrap), and the constitutive dPK1/d(F_stab) is
  // cauchy_jacobian = dsigma/d(dL) since d(dL)/d(F_stab) = I.
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    if (!need_jacobian)
      return;
    _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
    pullBackCauchyJacobian(_pk1_jacobian[_qp]);
  }
}
