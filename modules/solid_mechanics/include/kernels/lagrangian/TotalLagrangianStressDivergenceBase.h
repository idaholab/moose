//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LagrangianStressDivergenceBase.h"
#include "GradientOperator.h"

/// Enforce equilibrium with a total Lagrangian formulation
///
/// This class enforces equilibrium when used in conjunction with
/// the corresponding strain calculator (CalculateStrainLagrangianKernel)
/// and with either a stress calculator that provides the
/// 1st PK stress ("pk1_stress") and the derivative of the 1st PK stress
/// with respect to the deformation gradient ("pk1_jacobian")
///
/// This kernel should be used with the new "ComputeLagrangianStressBase"
/// stress update system and the "ComputeLagrangianStrain" system for strains.
///
template <class G>
class TotalLagrangianStressDivergenceBase : public LagrangianStressDivergenceBase, G
{
public:
  static InputParameters baseParams();
  static InputParameters validParams();
  TotalLagrangianStressDivergenceBase(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual RankTwoTensor gradTest(unsigned int component) override;
  virtual RankTwoTensor gradTrial(unsigned int component) override;
  virtual void precalculateJacobianDisplacement(unsigned int component) override;
  virtual void precalculateResidual() override;
  virtual void precalculateJacobian() override;
  virtual void precalculateOffDiagJacobian(unsigned int jvar) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobianDisplacement(unsigned int alpha, unsigned int beta) override;
  virtual Real computeQpJacobianTemperature(unsigned int cvar) override;
  virtual Real computeQpJacobianOutOfPlaneStrain() override;

  /// Compute element-averaged spatial gradient of test functions for component `_alpha`,
  /// filling `_avg_grad_spatial_test`. Used for the OLD-compat B-bar volumetric correction.
  void computeAverageGradientSpatialTest();

  /// Compute element-averaged spatial gradient of trial functions for component `beta`,
  /// filling `_avg_grad_spatial_phi[beta]`. Used for the B-bar Jacobian contribution.
  void computeAverageGradientSpatialPhi(unsigned int beta);

  /// Compute element-averaged cross product `(grad_x test_i)_{b1} * (grad_x phi_j)_{b2}` for
  /// `b1, b2 in {_alpha, beta}` (4 combinations), filling `_avg_test_phi_cross[b1][b2][i][j]`.
  /// Used by the non-local B-bar Jacobian contribution from d(avg_grad_spatial_test)/dU.
  /// `beta` is the displacement component of the trial-side variable in this Jacobian column;
  /// only the four (`_alpha`, `_alpha`), (`_alpha`, beta), (beta, `_alpha`), (beta, beta) entries
  /// are populated (others are left untouched / unread).
  void computeAvgTestPhiCross(unsigned int beta);

  /// (grad_x test_i)_component at the current `_qp`, where the push-forward uses
  /// the unstabilized F (= F_actual at alpha = 1), consistent with the PK1 wrap.
  Real gradXTestComponent(unsigned int component) const;

  /// (grad_x phi_j)_component at the current `_qp`, same push-forward as `gradXTestComponent`.
  Real gradXPhiComponent(unsigned int component) const;

  /// The 1st Piola-Kirchhoff stress
  const MaterialProperty<RankTwoTensor> & _pk1;

  /// The derivative of the PK1 stress with respect to the deformation gradient (F that the
  /// stress material consumed, i.e. the alpha-weighted F under the generalized midpoint rule).
  /// Used by the homogenization macro-var Jacobian path, which couples to F directly.
  const MaterialProperty<RankFourTensor> & _dpk1;

  /// The derivative of the PK1 stress with respect to the displacement gradient (grad u_{n+1}).
  /// Equals _dpk1 * _d_F_d_grad_u, populated in ComputeLagrangianStressBase::computeQpProperties.
  /// Used by the TL displacement Jacobian so the kernel does not need to know about the
  /// generalized-alpha kinematic policy.
  const MaterialProperty<RankFourTensor> & _dpk1_d_grad_u;

  /// Variant of `_dpk1` (= pk1_jacobian) computed WITHOUT the F-bar chain factor
  /// `_d_F_stab_d_F_ust` in the sigma-via-dL contribution. Used for coupled variables
  /// that add to `_F` AFTER F-bar runs (out-of-plane strain in WPS, homogenization
  /// macro_grad) -- those perturbations bypass F-bar's chain, so the sigma side of
  /// `dPK1/d(F_ust)` must NOT include the F-bar local correction.
  const MaterialProperty<RankFourTensor> & _dpk1_bypass_fbar;

protected:
  /// The unstabilized trial function gradient. Exposed (protected, not private) so
  /// subclasses like `HomogenizedTotalLagrangianStressDivergence` can use it directly
  /// for the macro-var <-> disp off-diagonal Jacobian chain.
  virtual RankTwoTensor gradTrialUnstabilized(unsigned int component);

private:
  /// The stabilized trial function gradient
  virtual RankTwoTensor gradTrialStabilized(unsigned int component);

  /// Populate the per-(qp, j) caches consumed by `computeQpJacobianDisplacement`. beta is fixed
  /// across a single Jacobian column, so the R4*R2 chain `_dpk1_d_grad_u[qp] * gradTrial(beta)`,
  /// the cached unstabilized trial gradient, and (when `_stabilize_strain == true`) the fully
  /// wrapped non-local F-bar PK1 perturbation can all be computed once per (qp, j) instead
  /// of once per (qp, _i, _j). Called from `precalculateJacobian` (with beta = `_alpha`) and
  /// `precalculateOffDiagJacobian(jvar)` (with the matched beta).
  void populateLocalPK1Cache(unsigned int beta);

  /// `_dpk1_d_grad_u[qp] * gradTrial(beta)` per (qp, j) for the current Jacobian column's beta.
  /// Used by the main local PK1 chain and (in `F_bar_mode = incremental`) the B-bar
  /// Jacobian's local PK1 derivative -- `gradTrial == gradTrialUnstabilized` in TL so a
  /// single cache covers both consumers.
  std::vector<std::vector<RankTwoTensor>> _dpk1_grad_trial_cache;

  /// Cached `gradTrialUnstabilized(beta)` per (qp, j) for the current column. Used by the
  /// B-bar Jacobian branch (`F_bar_mode = incremental`). Cheap to store and avoids the
  /// per-(_i, _j) `G::gradOp` call.
  std::vector<std::vector<RankTwoTensor>> _grad_trial_cache;

  /// Fully wrapped non-local F-bar contribution to deltaPK1 per (qp, j) for the current column:
  ///   _D_nl_cache[qp] * (_d_F_d_grad_u[qp] * _avg_grad_trial[beta][j])
  /// then PK1-wrapped via `_F_ust_det_cache[qp]` / `_F_ust_inv_T_cache[qp]` in large
  /// kinematics. Populated only when `_stabilize_strain == true`; replaces the per-call
  /// `deltaPK1NonLocalFBar(...)` invocation in TL's displacement Jacobian.
  std::vector<std::vector<RankTwoTensor>> _delta_PK1_NL_cache;
};
