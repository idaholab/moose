//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressCauchy.h"

InputParameters
ComputeLagrangianStressCauchy::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();

  params.addClassDescription("Stress update based on the Cauchy stress");

  return params;
}

ComputeLagrangianStressCauchy::ComputeLagrangianStressCauchy(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                     "inverse_incremental_deformation_gradient")),
    _inv_def_grad(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient"))
{
}

void
ComputeLagrangianStressCauchy::computeQpStressUpdate()
{
  computeQpCauchyStress();
  computeQpPK1Stress(); // This could be "switched"
}

void
ComputeLagrangianStressCauchy::computeQpPK1Stress()
{
  // Wrap sigma -> PK1 using the *unstabilized* F (= F_actual at alpha = 1). F-bar enters
  // only through sigma via the strain calc's F-bar'd `_f_inv`, NOT through this kinematic
  // wrap -- this matches OLD's `StressDivergenceTensors`-on-displaced-mesh residual,
  // which integrates `gradTest_spatial * sigma dV` on the actual deformed mesh (so the
  // implicit "wrap F" is F_actual, NOT the F-bar'd F).
  //
  // The R4 algebra for `_pk1_jacobian` and `_pk1_jacobian_bypass_fbar` is Jacobian-only;
  // the wrap to `_pk1_stress` is not. Gate accordingly.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();
  if (_large_kinematics)
  {
    const RankTwoTensor F_ust_inv = _F_ust[_qp].inverse();
    const RankTwoTensor F_ust_invT = F_ust_inv.transpose();
    const Real det_F_ust = _F_ust[_qp].det();
    _pk1_stress[_qp] = det_F_ust * _cauchy_stress[_qp] * F_ust_invT;

    if (!need_jacobian)
      return;

    usingTensorIndices(i_, j_, k_, l_);
    // Geometric pieces of dPK1/d(F_ust) (independent of F-bar):
    const RankFourTensor geom = _pk1_stress[_qp].outerProduct(F_ust_invT) -
                                _pk1_stress[_qp].times<i_, l_, j_, k_>(F_ust_inv);
    // sigma chain WITHOUT the F-bar `_d_F_stab_d_F_ust` factor -- for specialty kernels
    // whose coupled variable adds to F_ust AFTER F-bar (WPS strain_zz, homogenization
    // macro_grad), so the perturbation bypasses F-bar's chain.
    const RankFourTensor sigma_chain_no_fbar =
        det_F_ust *
        (_cauchy_jacobian[_qp] * _d_deformation_gradient_increment_d_F[_qp]).singleProductJ(F_ust_inv);
    _pk1_jacobian_bypass_fbar[_qp] = geom + sigma_chain_no_fbar;
    // sigma chain WITH the F-bar chain -- the default `_pk1_jacobian` for disp Jacobian.
    _pk1_jacobian[_qp] =
        geom + det_F_ust * (_cauchy_jacobian[_qp] * _d_deformation_gradient_increment_d_F[_qp] *
                            _d_F_stab_d_F_ust[_qp])
                               .singleProductJ(F_ust_inv);
  }
  else
  {
    // Small kinematics: PK1 = sigma (no wrap). The F-bar chain enters via
    // `_d_F_stab_d_F_ust` for the disp Jacobian path; specialty paths bypass it.
    _pk1_stress[_qp] = _cauchy_stress[_qp];
    if (!need_jacobian)
      return;
    _pk1_jacobian[_qp] = _cauchy_jacobian[_qp] * _d_F_stab_d_F_ust[_qp];
    _pk1_jacobian_bypass_fbar[_qp] = _cauchy_jacobian[_qp];
  }
}
