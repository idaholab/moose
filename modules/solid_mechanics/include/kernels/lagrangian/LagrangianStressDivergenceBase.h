//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelScalarBase.h"
#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "StabilizationUtils.h"
#include "RankFourTensorForward.h"
#include "GuaranteeConsumer.h"

/// Base class of the "Lagrangian" kernel system
///
/// This class provides a common structure for the "new" tensor_mechanics
/// kernel system.  The goals for this new system are
///   1) Always-correct jacobians
///   2) A cleaner material interface
///
/// This class provides common input properties and helper methods,
/// most of the math has to be done in the subclasses
///
class LagrangianStressDivergenceBase
  : public JvarMapKernelInterface<DerivativeMaterialInterface<KernelScalarBase>>,
    public GuaranteeConsumer
{
public:
  static InputParameters validParams();
  LagrangianStressDivergenceBase(const InputParameters & parameters);

  /// Mirrors `ComputeLagrangianStrainBase::FBarMode`. Replicated here (rather than included)
  /// so this header doesn't need to pull in the templated strain header.
  enum class FBarMode
  {
    Total,
    Incremental
  };

protected:
  // Helper function to return the test function gradient which may depend on kinematics and
  // stabilization
  virtual RankTwoTensor gradTest(unsigned int component) = 0;

  // Helper function to return the trial function gradient which may depend on kinematics and
  // stabilization
  virtual RankTwoTensor gradTrial(unsigned int component) = 0;

  /// Derive `_large_kinematics` from the strain calculator's LARGE_KINEMATICS guarantee
  virtual void initialSetup() override;

  virtual void precalculateJacobian() override;
  virtual void precalculateOffDiagJacobian(unsigned int jvar) override;

  /// Prepare the average shape function gradients for stabilization
  virtual void precalculateJacobianDisplacement(unsigned int component) = 0;

  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // Derivatives of the residual w.r.t. the displacement dofs
  virtual Real computeQpJacobianDisplacement(unsigned int alpha, unsigned int beta) = 0;

  // Derivatives of the residual w.r.t. the temperature dofs through eigenstrain
  virtual Real computeQpJacobianTemperature(unsigned int cvar) = 0;

  // Derivatives of the residual w.r.t. the out-of-plane strain
  virtual Real computeQpJacobianOutOfPlaneStrain() = 0;

  /// Non-local F-bar contribution to deltaPK1 at the current `_qp`, given the perturbation
  /// `delta_F_avg` of the element-average F. Implements the shared chain
  ///   deltaF_stab_NL = _d_F_stab_d_F_avg * deltaF_avg
  ///   deltadL_NL    = _d_deformation_gradient_increment_d_F * deltaF_stab_NL
  ///   deltasigma_NL     = _cauchy_jacobian * deltadL_NL
  ///   deltaPK1_NL   = det(F_ust) * deltasigma_NL * F_ust^{-T}    (large kinematics)
  ///             = deltasigma_NL                               (small kinematics, PK1 == sigma)
  /// Returns zero when F-bar is off (`!_stabilize_strain`). Used by the TL displacement
  /// Jacobian, the WPS off-diag Jacobian, and the homogenization scalar<->disp Jacobian
  /// -- anywhere the disp perturbation chains through F-bar's non-local route.
  ///
  /// Consumes the strain/stress material's per-qp `_d_nl_fbar` operator so the inner
  /// (test, trial) loop pays only one R4*R2 (and, in the large-kinematics branch, one
  /// R2*R2*R2 PK1 wrap) per call instead of three R4*R2 chains plus a per-call 3x3 inverse.
  RankTwoTensor deltaPK1NonLocalFBar(const RankTwoTensor & delta_F_avg) const;

protected:
  /// If true use large deformation kinematics. Derived in initialSetup() from the strain
  /// calculator's LARGE_KINEMATICS guarantee (single source of truth); the kernel's own
  /// `large_kinematics` parameter is deprecated.
  bool _large_kinematics;

  /// If true calculate the deformation gradient derivatives for F_bar
  const bool _stabilize_strain;

  /// What F gets F-bar volumetric correction (Total vs. Incremental). Must match the strain
  /// calc's `F_bar_mode`. Incremental mode changes how `_avg_grad_trial` is computed so the
  /// non-local F-bar chain captures deltaf_avg (= avg of deltaF_ust * F_ust_old^{-1}) instead of deltaF_avg.
  const FBarMode _F_bar_mode;

  /// Prepend to the material properties
  const std::string _base_name;

  /// Which component of the vector residual this kernel is responsible for
  const unsigned int _alpha;

  /// Total number of displacements/size of residual vector
  const unsigned int _ndisp;

  /// The displacement numbers
  std::vector<unsigned int> _disp_nums;

  // Averaged trial function gradients for each displacement component
  // i.e. _avg_grad_trial[a][j] returns the average gradient of trial function associated with
  // node j with respect to displacement component a.
  std::vector<std::vector<RankTwoTensor>> _avg_grad_trial;

  /// Element-averaged spatial (deformed-frame) gradient of test functions for the kernel's
  /// component `_alpha`:
  ///   _avg_grad_spatial_test[i] = (1/V_x) int_e (grad_x test_i)_alpha dV_x
  /// Used for OLD-compat B-bar volumetric correction in `F_bar_mode = incremental`.
  /// Populated by the TL kernel's `precalculateResidual` / `precalculateJacobian` overrides
  /// when `_stabilize_strain == true`. The trial-function analog is published per (component, j)
  /// in `_avg_grad_spatial_phi` and used by the Jacobian.
  std::vector<Real> _avg_grad_spatial_test;

  /// Element-averaged spatial gradient of trial functions per component:
  ///   _avg_grad_spatial_phi[component][j] = (1/V_x) int_e (grad_x phi_j)_component dV_x
  /// Populated alongside `_avg_grad_spatial_test` for the Jacobian's B-bar contribution.
  std::vector<std::vector<Real>> _avg_grad_spatial_phi;

  /// Element-averaged cross product (on displaced mesh) of spatial test/trial gradients:
  ///   _avg_test_phi_cross[b1][b2][i][j] = (1/V_x) int_e (grad_x test_i)_{b1} * (grad_x phi_j)_{b2} dV_x
  /// Indexed by [b1][b2] in {0,1,2}^2. Only the (b1, b2) entries needed for the current Jacobian
  /// call are populated by the TL kernel's `computeAvgTestPhiCross(beta)` helper. Used for the
  /// B-bar Jacobian's non-local term (d(avg_grad_spatial_test)/dU).
  std::vector<std::vector<std::vector<std::vector<Real>>>> _avg_test_phi_cross;

  /// The unmodified deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_ust;

  /// Old unstabilized deformation gradient. Only consulted in `F_bar_mode = incremental` for
  /// the kernel's element-average of grad_phi * F_ust_old^{-1}. Always fetched (cheap) so the
  /// kernel doesn't need conditional property bookkeeping.
  const MaterialProperty<RankTwoTensor> & _F_ust_old;

  /// The element-average deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_avg;

  /// The inverse increment deformation gradient
  const MaterialProperty<RankTwoTensor> & _f_inv;

  /// The inverse deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_inv;

  /// F_ust^{-1} and det(F_ust) from the strain calculator, consumed by the F-bar spatial
  /// push-forward (grad_x = F_ust^{-T} grad_X, J_ust = det F_ust) on both the residual and
  /// Jacobian sweeps. Fetched only when `_stabilize_strain` (nullptr otherwise). The strain
  /// material computes them once per qp (shared by all displacement kernels), replacing the
  /// former per-kernel F_ust-inverse/det cache and the per-test/trial recomputation.
  const MaterialProperty<RankTwoTensor> * _F_ust_inv = nullptr;
  const MaterialProperty<Real> * _F_ust_det = nullptr;

  /// The actual (stabilized) deformation gradient. With the generalized midpoint rule this is the
  /// alpha-weighted F, NOT the literal F at n+1.
  const MaterialProperty<RankTwoTensor> & _F;

  /// The literal deformation gradient at n+1 (I + grad u_{n+1}), independent of alpha and F-bar.
  /// Used by the UL kernel to convert spatial gradients to reference-frame gradients.
  const MaterialProperty<RankTwoTensor> & _F_actual;

  /// Derivative of the spatial velocity gradient increment w.r.t. F_{n+1}
  const MaterialProperty<RankFourTensor> & _d_deformation_gradient_increment_d_F;

  /// Derivative of F_{n+1} w.r.t. the displacement gradient
  const MaterialProperty<RankFourTensor> & _d_F_d_grad_u;

  /// Partials of the F-bar-stabilized deformation gradient. Used by the UL kernel to assemble
  /// the F-bar Jacobian contribution. For F-bar off, _d_F_stab_d_F_ust = I^(4) and
  /// _d_F_stab_d_F_avg = 0, so the kernel chain reduces to the unstabilized case. TL also
  /// uses `_d_F_stab_d_F_avg` directly for the non-local F-bar Jacobian contribution to PK1
  /// (since after the F_ust-wrap architectural change PK1 = det(F_ust) sigma F_ust^{-T} no
  /// longer contains the non-local F-bar effect -- it has to enter through the sigma-via-dL
  /// chain explicitly).
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_ust;
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_avg;

  /// Temperature, if provided.  This is used only to get the trial functions
  const MooseVariable * _temperature;

  /// Out-of-plane strain, if provided.
  const MooseVariable * _out_of_plane_strain;

  /// Eigenstrain derivatives wrt generate coupleds
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *>> _deigenstrain_dargs;

  /// Derivative of the Cauchy stress with respect to the eigenstrain (published by
  /// ComputeLagrangianObjectiveStress; can be overridden by other Cauchy-providing
  /// materials). Only fetched when eigenstrains are coupled, since only the temperature
  /// off-diagonal Jacobian needs it. Will be nullptr otherwise; subclasses that consume
  /// it must guard accordingly.
  const MaterialProperty<RankFourTensor> * _dcauchy_stress_d_eigenstrain = nullptr;

  /// Composed non-local F-bar operator D_nl = cauchy_jac : d(dL)/dF : d(F_stab)/d(F_avg),
  /// published per qp by the stress material (once per element, shared by all displacement
  /// kernels). `deltaPK1NonLocalFBar(deltaF_avg)` collapses to one R4*R2 contraction against
  /// it. Fetched only when `_stabilize_strain` (nullptr otherwise).
  const MaterialProperty<RankFourTensor> * _d_nl_fbar = nullptr;
};
