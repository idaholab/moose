//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianStressDivergenceBase.h"

template <class G>
InputParameters
TotalLagrangianStressDivergenceBase<G>::baseParams()
{
  InputParameters params = LagrangianStressDivergenceBase::validParams();
  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

template <class G>
TotalLagrangianStressDivergenceBase<G>::TotalLagrangianStressDivergenceBase(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _pk1(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _dpk1(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian")),
    _dpk1_d_grad_u(getMaterialPropertyByName<RankFourTensor>(_base_name + "dpk1_d_grad_u")),
    _dpk1_bypass_fbar(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian_bypass_fbar"))
{
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTest(unsigned int component)
{
  // F-bar doesn't modify the test function
  return G::gradOp(component, _grad_test[_i][_qp], _test[_i][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrial(unsigned int component)
{
  // After the F_ust-wrap architectural change, pk1_jacobian = dPK1/d(F_ust) already
  // contains the local F-bar contribution (via the σ-chain through
  // `_d_F_stab_d_F_ust`). The trial gradient is always unstabilized; the non-local F-bar
  // Jacobian contribution is added explicitly in `computeQpJacobianDisplacement`.
  return gradTrialUnstabilized(component);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrialUnstabilized(unsigned int component)
{
  // Without F-bar stabilization, simply return the gradient of the trial functions
  return G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrialStabilized(unsigned int component)
{
  // The base unstabilized trial function gradient
  const auto Gb = G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
  // The average trial function gradient
  const auto Ga = _avg_grad_trial[component][_j];

  // The F-bar stabilization depends on kinematics
  if (_large_kinematics)
  {
    // Horrible thing, see the documentation for how we get here
    const Real dratio = std::pow(_F_avg[_qp].det() / _F_ust[_qp].det(), 1.0 / 3.0);
    const Real fact = (_F_avg[_qp].inverse().transpose().doubleContraction(Ga) -
                       _F_ust[_qp].inverse().transpose().doubleContraction(Gb)) /
                      3.0;
    return dratio * (Gb + fact * _F_ust[_qp]);
  }

  // The small kinematics modification is linear
  return Gb + (Ga.trace() - Gb.trace()) / 3.0 * RankTwoTensor::Identity();
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::precalculateJacobianDisplacement(unsigned int component)
{
  // For total Lagrangian, the averaging is taken on the reference frame regardless of geometric
  // nonlinearity. The stored quantity is what the strain calc's `_d_F_stab_d_F_avg` contracts
  // with: δF_avg in `F_bar_mode = total`, δf_avg in `F_bar_mode = incremental`.
  //   total:        avg_q(grad_phi[j][q])           → δF_avg per DOF
  //   incremental:  avg_q(grad_phi[j][q] · F_ust_old[q]^{-1})
  //                                                  → δf_avg per DOF
  //                 (since f_ust = F_ust · F_ust_old^{-1} and F_ust_old is fixed w.r.t. the
  //                  current Newton iterate's disp, so δf_ust = δF_ust · F_ust_old^{-1}).
  const bool incremental = (_F_bar_mode == FBarMode::Incremental);
  for (auto j : make_range(_phi.size()))
    _avg_grad_trial[component][j] = StabilizationUtils::elementAverage(
        [this, component, j, incremental](unsigned int qp)
        {
          const RankTwoTensor g = G::gradOp(component, _grad_phi[j][qp], _phi[j][qp], _q_point[qp]);
          return incremental ? g * _F_ust_old[qp].inverse() : g;
        },
        _JxW,
        _coord);
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::precalculateResidual()
{
  LagrangianStressDivergenceBase::precalculateResidual();
  if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
    computeAverageGradientSpatialTest();
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::precalculateJacobian()
{
  LagrangianStressDivergenceBase::precalculateJacobian();
  // Populate the per-(qp, j) local PK1 chain cache for the diagonal column (β = _alpha).
  // Runs regardless of `_stabilize_strain` because the local chain is always needed; the
  // helper itself only touches `_avg_grad_trial` when stab is on.
  populateLocalPK1Cache(_alpha);
  if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
  {
    computeAverageGradientSpatialTest();
    computeAverageGradientSpatialPhi(_alpha);
    computeAvgTestPhiCross(_alpha);
  }
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::precalculateOffDiagJacobian(unsigned int jvar)
{
  LagrangianStressDivergenceBase::precalculateOffDiagJacobian(jvar);
  // Populate the per-(qp, j) local PK1 chain cache for the matched off-diagonal β.
  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_nums[beta])
    {
      populateLocalPK1Cache(beta);
      if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
      {
        computeAverageGradientSpatialTest();
        computeAverageGradientSpatialPhi(beta);
        computeAvgTestPhiCross(beta);
      }
      break;
    }
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::populateLocalPK1Cache(unsigned int beta)
{
  const unsigned int n_qp = _qrule->n_points();
  const unsigned int n_phi = _phi.size();
  _dpk1_grad_trial_cache.assign(n_qp, std::vector<RankTwoTensor>(n_phi));
  _grad_trial_cache.assign(n_qp, std::vector<RankTwoTensor>(n_phi));
  if (_stabilize_strain)
    _delta_PK1_NL_cache.assign(n_qp, std::vector<RankTwoTensor>(n_phi));

  for (unsigned int qp = 0; qp < n_qp; ++qp)
  {
    const RankFourTensor & dpk1 = _dpk1_d_grad_u[qp];
    const RankFourTensor & dFdGU = _d_F_d_grad_u[qp];
    for (unsigned int j = 0; j < n_phi; ++j)
    {
      // gradTrial == gradTrialUnstabilized in TL (line ~50); single cache feeds both consumers.
      const RankTwoTensor grad_trial =
          G::gradOp(beta, _grad_phi[j][qp], _phi[j][qp], _q_point[qp]);
      _grad_trial_cache[qp][j] = grad_trial;
      _dpk1_grad_trial_cache[qp][j] = dpk1 * grad_trial;
      if (_stabilize_strain)
      {
        // Compose the full non-local F-bar PK1 perturbation:
        //   δσ_NL = _D_nl_cache[qp] · (_d_F_d_grad_u[qp] · _avg_grad_trial[β][j])
        //   δPK1_NL = det(F_ust) · δσ_NL · F_ust^{-T}   (large kinematics)
        //           = δσ_NL                              (small kinematics)
        // `_D_nl_cache` / `_F_ust_det_cache` / `_F_ust_inv_T_cache` are refreshed in
        // `prepareFBarCaches()` which runs in `LagrangianStressDivergenceBase::precalculate*`
        // before this method is called.
        const RankTwoTensor delta_F_avg = dFdGU * _avg_grad_trial[beta][j];
        const RankTwoTensor delta_sigma_nl = _D_nl_cache[qp] * delta_F_avg;
        if (_large_kinematics)
          _delta_PK1_NL_cache[qp][j] =
              _F_ust_det_cache[qp] * delta_sigma_nl * _F_ust_inv_T_cache[qp];
        else
          _delta_PK1_NL_cache[qp][j] = delta_sigma_nl;
      }
    }
  }
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::gradXTestComponent(unsigned int component) const
{
  // (grad_x test_i)_component = (grad_X test_i)_j (F_ust^{-1})_{j, component}
  // Push-forward via F_ust (= F_actual at alpha=1) matches the PK1 wrap.
  if (!_large_kinematics)
    return _grad_test[_i][_qp](component);
  const RankTwoTensor F_inv = _F_ust[_qp].inverse();
  Real out = 0.0;
  for (unsigned int j = 0; j < 3; ++j)
    out += _grad_test[_i][_qp](j) * F_inv(j, component);
  return out;
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::gradXPhiComponent(unsigned int component) const
{
  if (!_large_kinematics)
    return _grad_phi[_j][_qp](component);
  const RankTwoTensor F_inv = _F_ust[_qp].inverse();
  Real out = 0.0;
  for (unsigned int j = 0; j < 3; ++j)
    out += _grad_phi[_j][_qp](j) * F_inv(j, component);
  return out;
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::computeAverageGradientSpatialTest()
{
  // _avg_grad_spatial_test[i] = (1/V_x) ∫ (grad_x test_i)_alpha dV_x
  //                           = (∑_qp (grad_X test_i)_j (F_ust^{-1})_{j, _alpha} · J_ust · w)
  //                             / (∑_qp J_ust · w),  w = JxW · coord
  // For small kinematics, F_ust = I, J_ust = 1, reducing to OLD's element-averaged grad_test.
  _avg_grad_spatial_test.assign(_test.size(), 0.0);
  Real V_x = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const Real J = _large_kinematics ? _F_ust[qp].det() : 1.0;
    const Real w = J * _JxW[qp] * _coord[qp];
    V_x += w;
    const RankTwoTensor F_inv =
        _large_kinematics ? _F_ust[qp].inverse() : RankTwoTensor::Identity();
    for (unsigned int i = 0; i < _test.size(); ++i)
    {
      Real g_x = 0.0;
      for (unsigned int j = 0; j < 3; ++j)
        g_x += _grad_test[i][qp](j) * F_inv(j, _alpha);
      _avg_grad_spatial_test[i] += g_x * w;
    }
  }
  for (unsigned int i = 0; i < _test.size(); ++i)
    _avg_grad_spatial_test[i] /= V_x;
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::computeAverageGradientSpatialPhi(unsigned int beta)
{
  // Same structure as computeAverageGradientSpatialTest, but for trial functions and component
  // beta.
  _avg_grad_spatial_phi[beta].assign(_phi.size(), 0.0);
  Real V_x = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const Real J = _large_kinematics ? _F_ust[qp].det() : 1.0;
    const Real w = J * _JxW[qp] * _coord[qp];
    V_x += w;
    const RankTwoTensor F_inv =
        _large_kinematics ? _F_ust[qp].inverse() : RankTwoTensor::Identity();
    for (unsigned int j = 0; j < _phi.size(); ++j)
    {
      Real g_x = 0.0;
      for (unsigned int k = 0; k < 3; ++k)
        g_x += _grad_phi[j][qp](k) * F_inv(k, beta);
      _avg_grad_spatial_phi[beta][j] += g_x * w;
    }
  }
  for (unsigned int j = 0; j < _phi.size(); ++j)
    _avg_grad_spatial_phi[beta][j] /= V_x;
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::computeAvgTestPhiCross(unsigned int beta)
{
  // Populate the cross-products
  //   _avg_test_phi_cross[b1][b2][i][j] = (1/V_x) ∫ (grad_x test_i)_{b1} · (grad_x phi_j)_{b2} dV_x
  // for the 4 combinations of (b1, b2) ∈ {_alpha, beta}. The d(avg_grad_spatial_test)/dU
  // term in the B-bar Jacobian needs both (alpha, beta) and (beta, alpha) entries; the
  // diagonal (alpha == beta) collapses to a single combination.
  const unsigned int n_test = _test.size();
  const unsigned int n_phi = _phi.size();
  std::vector<unsigned int> bs = {_alpha};
  if (beta != _alpha)
    bs.push_back(beta);
  // Zero out and resize the affected slots
  for (auto b1 : bs)
    for (auto b2 : bs)
    {
      _avg_test_phi_cross[b1][b2].assign(n_test, std::vector<Real>(n_phi, 0.0));
    }
  Real V_x = 0.0;
  // Cache (grad_x test_i)_{b1} and (grad_x phi_j)_{b2} per qp to avoid recomputing for
  // each (i, j) pair.
  std::vector<std::vector<Real>> gx_test(bs.size(), std::vector<Real>(n_test, 0.0));
  std::vector<std::vector<Real>> gx_phi(bs.size(), std::vector<Real>(n_phi, 0.0));
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const Real J = _large_kinematics ? _F_ust[qp].det() : 1.0;
    const Real w = J * _JxW[qp] * _coord[qp];
    V_x += w;
    const RankTwoTensor F_inv =
        _large_kinematics ? _F_ust[qp].inverse() : RankTwoTensor::Identity();
    for (unsigned int bi = 0; bi < bs.size(); ++bi)
    {
      const unsigned int b = bs[bi];
      for (unsigned int i = 0; i < n_test; ++i)
      {
        Real g = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          g += _grad_test[i][qp](k) * F_inv(k, b);
        gx_test[bi][i] = g;
      }
      for (unsigned int j = 0; j < n_phi; ++j)
      {
        Real g = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          g += _grad_phi[j][qp](k) * F_inv(k, b);
        gx_phi[bi][j] = g;
      }
    }
    for (unsigned int b1i = 0; b1i < bs.size(); ++b1i)
      for (unsigned int b2i = 0; b2i < bs.size(); ++b2i)
      {
        const unsigned int b1 = bs[b1i];
        const unsigned int b2 = bs[b2i];
        auto & dest = _avg_test_phi_cross[b1][b2];
        for (unsigned int i = 0; i < n_test; ++i)
          for (unsigned int j = 0; j < n_phi; ++j)
            dest[i][j] += gx_test[b1i][i] * gx_phi[b2i][j] * w;
      }
  }
  for (auto b1 : bs)
    for (auto b2 : bs)
    {
      auto & dest = _avg_test_phi_cross[b1][b2];
      for (unsigned int i = 0; i < n_test; ++i)
        for (unsigned int j = 0; j < n_phi; ++j)
          dest[i][j] /= V_x;
    }
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpResidual()
{
  Real result = gradTest(_alpha).doubleContraction(_pk1[_qp]);

  // OLD-compat B-bar volumetric correction (only in `F_bar_mode = incremental`, the explicit
  // OLD-compat mode). This adds
  //   ∫ (tr σ / 3) · (avg_grad_x test - grad_x test)_alpha dV_x
  // expressed in reference-frame integration as
  //   ∑_qp ((PK1 : F_ust) / 3) · (avg_grad_spatial_test[_i] - grad_x_test_alpha) · JxW · coord
  // (the J = det(F_ust) factor is already absorbed into `PK1 : F_ust = J · tr σ`).
  // Without this term, PK1 = det(F_ust) σ F_ust^{-T} reproduces OLD's `σ : grad_x test` term
  // but not OLD's vol-locking correction; for uniform σ that's harmless (the integral vanishes
  // per element) but for non-uniform σ — e.g. mixed-component Dirichlet BCs producing non-affine
  // equilibrium with plasticity + a kinematic decomposition that gives spatially varying σ —
  // it drives the converged displacement away from OLD's. Scoped to incremental mode because
  // (a) that's the documented OLD-compat F-bar mode and (b) the existing `total` mode tests
  // are calibrated to the F-bar-in-σ-only formulation (adding the B-bar term there would be
  // a separate behavior change).
  if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
  {
    const Real PK1_F_over_3 = _pk1[_qp].doubleContraction(_F_ust[_qp]) / 3.0;
    result += PK1_F_over_3 * (_avg_grad_spatial_test[_i] - gradXTestComponent(_alpha));
  }
  return result;
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianDisplacement(unsigned int alpha,
                                                                      unsigned int beta)
{
  // Cache gradTest(α) — used twice below, cheap-but-not-free `G::gradOp`.
  const RankTwoTensor grad_test = gradTest(alpha);

  // Local Jacobian: J_{alpha beta} = gradTest_α : (dPK1/d(grad u) · grad_phi_β). The
  // R4·R2 chain `_dpk1_d_grad_u[qp] · gradTrial(β)` depends only on (qp, j); it was
  // precomputed once per (qp, j) by `populateLocalPK1Cache(β)` in the column-level
  // precalculate hook.
  Real J = grad_test.doubleContraction(_dpk1_grad_trial_cache[_qp][_j]);

  // Non-local F-bar Jacobian contribution. The fully wrapped δPK1_NL per (qp, j) is
  // precomputed in `populateLocalPK1Cache(β)` (it depends only on (qp, j) since β is
  // fixed for the column) — no per-(_i, _j) R4·R2 chain or 3x3 inverse here.
  RankTwoTensor d_PK1_NL;
  if (_stabilize_strain)
  {
    d_PK1_NL = _delta_PK1_NL_cache[_qp][_j];
    J += grad_test.doubleContraction(d_PK1_NL);
  }

  // OLD-compat B-bar volumetric correction Jacobian (only in `F_bar_mode = incremental`).
  // The residual added the term R_extra = (PK1:F_ust)/3 · (avg_T - T_α) summed over qps.
  // Linearizing per (i, j, α, β):
  //   J_extra_qp = (1/3) · d(PK1:F_ust)/dU · (avg_T - T_α)
  //              + (1/3) · (PK1:F_ust) · (d(avg_T)/dU - dT_α/dU)
  // with derivatives in U (the trial-DOF for component β at node j):
  //   d(PK1:F_ust)/dU|qp = (dPK1_local + dPK1_NL) : F_ust + PK1 : dF_ust_local
  //     dPK1_local = _dpk1_grad_trial_cache[_qp][_j]      (cached above)
  //     dPK1_NL    = d_PK1_NL                             (cached above)
  //     dF_ust     = _grad_trial_cache[_qp][_j]           (cached unstabilized trial)
  //   dT_α/dU|qp  = -(grad_x test_i)_β · (grad_x phi_j)_α            (local cross)
  //   d(avg_T)/dU = avgCross(α, β) - avgCross(β, α) - avg_T(i) · avg_grad_spatial_phi[β][j]
  //     avgCross(b1, b2) = _avg_test_phi_cross[b1][b2][i][j]         (precomputed)
  if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
  {
    const Real A_qp = _pk1[_qp].doubleContraction(_F_ust[_qp]) / 3.0;
    const Real avg_T = _avg_grad_spatial_test[_i];
    const Real T_alpha = gradXTestComponent(alpha);
    const Real B = avg_T - T_alpha;

    // dA/dU at this qp: combine local + non-local PK1 deriv, plus PK1 : dF_ust local.
    const RankTwoTensor & d_PK1_local = _dpk1_grad_trial_cache[_qp][_j];
    const RankTwoTensor & d_F_ust = _grad_trial_cache[_qp][_j];
    const Real dA_dU = ((d_PK1_local + d_PK1_NL).doubleContraction(_F_ust[_qp]) +
                        _pk1[_qp].doubleContraction(d_F_ust)) /
                       3.0;

    // dB/dU: -dT_α/dU + d(avg_T)/dU
    // dT_α/dU = -(grad_x test_i)_β · (grad_x phi_j)_α  (local cross at _qp)
    const Real T_beta_i = gradXTestComponent(beta);
    const Real phi_alpha_j = gradXPhiComponent(alpha);
    const Real dT_alpha_dU = -T_beta_i * phi_alpha_j;

    const Real avg_cross_ab = _avg_test_phi_cross[alpha][beta][_i][_j];
    const Real avg_cross_ba = _avg_test_phi_cross[beta][alpha][_i][_j];
    const Real d_avg_T_dU = avg_cross_ab - avg_cross_ba - avg_T * _avg_grad_spatial_phi[beta][_j];

    const Real dB_dU = d_avg_T_dU - dT_alpha_dU;
    J += dA_dU * B + A_qp * dB_dU;
  }

  return J;
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianTemperature(unsigned int cvar)
{
  // Multiple eigenstrains may depend on the same coupled var
  RankTwoTensor total_deigen;
  for (const auto deigen_darg : _deigenstrain_dargs[cvar])
    total_deigen += (*deigen_darg)[_qp];

  // No eigenstrain → no temperature coupling. Short-circuit before dereferencing the
  // d_sigma/d_eigenstrain property (only fetched when eigenstrains are coupled; see
  // LagrangianStressDivergenceBase ctor).
  if (total_deigen.L2norm() == 0.0)
    return 0.0;

  // Direct chain through the constitutive update. The stress material publishes
  //   d_sigma/d_eigenstrain   (= -Jinv * small_jacobian for the objective-rate path,
  //                            with the sign convention that an eigenstrain increase
  //                            reduces the mechanical strain).
  // We then wrap to PK1 the same way the residual does:
  //   dP/dT = det(F) * dsigma/dT * F^{-T}   (large kinematics)
  //   dP/dT = dsigma/dT                     (small kinematics, P == sigma by convention)
  const RankTwoTensor dsigma_dT = (*_dcauchy_stress_d_eigenstrain)[_qp] * total_deigen;
  RankTwoTensor dP_dT;
  if (_large_kinematics)
    dP_dT = _F[_qp].det() * dsigma_dT * _F_inv[_qp].transpose();
  else
    dP_dT = dsigma_dT;
  return dP_dT.doubleContraction(gradTest(_alpha)) * _temperature->phi()[_j][_qp];
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianOutOfPlaneStrain()
{
  // d(R_disp_α)/d(strain_zz_j) at qp = gradTest_α : d(PK1)/d(strain_zz_j).
  // strain_zz feeds `_F[(2,2)]` AFTER F-bar runs in `ComputeLagrangianWPSStrain`, so
  // strain_zz perturbations bypass F-bar's chain. Use `_dpk1_bypass_fbar` (= the
  // pk1_jacobian variant computed with the F-bar `_d_F_stab_d_F_ust` factor REPLACED
  // by identity in the σ chain) for a consistent Jacobian.
  return _dpk1_bypass_fbar[_qp].contractionKl(2, 2, gradTest(_alpha)) *
         _out_of_plane_strain->phi()[_j][_qp];
}

template class TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorCentrosymmetricSpherical>;
