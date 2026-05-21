//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangianObjectiveRate.h"
#include "ComputeLagrangianObjectiveStress.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

// ============================================================================
// Linear-template shared helpers
// ============================================================================

std::tuple<RankTwoTensor, RankFourTensor>
LagrangianLinearObjectiveRate::advectStress(const RankTwoTensor & S0,
                                            const RankTwoTensor & dQ)
{
  const RankFourTensor J = updateTensor(dQ);
  const RankFourTensor Jinv = J.inverse();
  const RankTwoTensor S = Jinv * S0;
  return {S, Jinv};
}

RankFourTensor
LagrangianLinearObjectiveRate::updateTensor(const RankTwoTensor & dQ)
{
  const auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return (1.0 + dQ.trace()) * I.times<i, k, j, l>(I) - dQ.times<i, k, j, l>(I) -
         I.times<i, k, j, l>(dQ);
}

RankFourTensor
LagrangianLinearObjectiveRate::stressAdvectionDerivative(const RankTwoTensor & S)
{
  const auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return S.times<i, j, k, l>(I) - I.times<i, k, l, j>(S) - S.times<i, l, j, k>(I);
}

RankFourTensor
LagrangianLinearObjectiveRate::cauchyJacobian(const RankFourTensor & Jinv,
                                              const RankFourTensor & small_jacobian,
                                              const RankFourTensor & U)
{
  return Jinv * (small_jacobian - U);
}

// ============================================================================
// Truesdell
// ============================================================================

void
LagrangianTruesdellRate::update(ComputeLagrangianObjectiveStress & host,
                                const RankTwoTensor & dS) const
{
  const unsigned int qp = host._qp;
  const RankTwoTensor & dL = host._spatial_velocity_increment[qp];

  auto [S, Jinv] = advectStress(host._cauchy_stress_old[qp] + dS, dL);
  host._dcauchy_stress_d_eigenstrain[qp] = -Jinv * host._small_jacobian[qp];

  const RankFourTensor U = stressAdvectionDerivative(S);
  host._cauchy_jacobian[qp] = cauchyJacobian(Jinv, host._small_jacobian[qp], U);
  host._cauchy_stress[qp] = S;
}

// ============================================================================
// Jaumann
// ============================================================================

void
LagrangianJaumannRate::update(ComputeLagrangianObjectiveStress & host,
                              const RankTwoTensor & dS) const
{
  const unsigned int qp = host._qp;
  const RankTwoTensor & dW = host._vorticity_increment[qp];

  auto [S, Jinv] = advectStress(host._cauchy_stress_old[qp] + dS, dW);
  host._dcauchy_stress_d_eigenstrain[qp] = -Jinv * host._small_jacobian[qp];

  const RankFourTensor d_dW_d_dL =
      host._d_vorticity_increment_d_F[qp] *
      host._d_spatial_velocity_increment_d_F[qp].inverse();
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dW_d_dL;
  host._cauchy_jacobian[qp] = cauchyJacobian(Jinv, host._small_jacobian[qp], U);
  host._cauchy_stress[qp] = S;
}

// ============================================================================
// Green-Naghdi
// ============================================================================

void
LagrangianGreenNaghdiRate::update(ComputeLagrangianObjectiveStress & host,
                                  const RankTwoTensor & dS) const
{
  usingTensorIndices(i, j, k, l, m);
  const unsigned int qp = host._qp;

  const RankTwoTensor I = RankTwoTensor::Identity();
  const RankTwoTensor dR = host._rotation[qp] * host._rotation_old[qp].transpose() - I;
  const RankTwoTensor dO = dR * host._inv_df[qp];

  auto [S, Jinv] = advectStress(host._cauchy_stress_old[qp] + dS, dO);
  host._dcauchy_stress_d_eigenstrain[qp] = -Jinv * host._small_jacobian[qp];

  const RankFourTensor & d_R_d_F = host._d_rotation_d_F[qp];
  const RankFourTensor d_F_d_dL = host._d_spatial_velocity_increment_d_F[qp].inverse();
  const RankTwoTensor T = host._rotation_old[qp].transpose() * host._inv_df[qp];

  const RankFourTensor d_invdf_d_F =
      -host._inv_df[qp].times<i, k, l, j>(host._inv_def_grad[qp]);
  const RankFourTensor d_invdf_d_dL = d_invdf_d_F * d_F_d_dL;
  const RankFourTensor d_dO_d_invdf = dR.times<i, k, j, l>(I);

  const RankFourTensor d_dO_d_dL =
      T.times<m, j, i, m, k, l>(d_R_d_F * d_F_d_dL) + d_dO_d_invdf * d_invdf_d_dL;
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dO_d_dL;
  host._cauchy_jacobian[qp] = cauchyJacobian(Jinv, host._small_jacobian[qp], U);
  host._cauchy_stress[qp] = S;
}

// ============================================================================
// Rashid
// ============================================================================

RankTwoTensor
LagrangianRashidRate::rotationFromVorticity(const RankTwoTensor & W,
                                            RankFourTensor & dR_dW)
{
  // For a skew W in 3D, θ = √((W : W) / 2). R = exp(W) via Rodrigues:
  //   R = I + f(θ) W + g(θ) W²,  f = sin θ / θ,  g = (1 − cos θ) / θ².
  // For θ → 0, fall back to the Taylor expansion: R ≈ I + W + W²/2.
  const Real theta2 = 0.5 * W.doubleContraction(W);
  const Real theta = std::sqrt(theta2);
  const auto I2 = RankTwoTensor::Identity();
  const RankTwoTensor W2 = W * W;

  Real f, g, df_dth, dg_dth;
  const Real small_theta = 1.0e-7;
  if (theta < small_theta)
  {
    f = 1.0 - theta2 / 6.0;
    g = 0.5 - theta2 / 24.0;
    df_dth = -theta / 3.0;
    dg_dth = -theta / 12.0;
  }
  else
  {
    const Real s = std::sin(theta), c = std::cos(theta);
    f = s / theta;
    g = (1.0 - c) / theta2;
    df_dth = (theta * c - s) / theta2;
    dg_dth = (theta * s - 2.0 * (1.0 - c)) / (theta * theta2);
  }
  const RankTwoTensor R = I2 + f * W + g * W2;

  // dR_ij/dW_mn = (df/dθ · dθ/dW_mn) W_ij + f · δ_im δ_jn
  //             + (dg/dθ · dθ/dW_mn) (W²)_ij + g · (δ_im W_nj + W_im δ_jn)
  // dθ/dW_mn = W_mn / (2θ)   (from d(θ²)/dW = W).
  usingTensorIndices(i_, j_, m_, n_);
  const RankFourTensor d_W2_dW =
      I2.template times<i_, m_, n_, j_>(W) + W.template times<i_, m_, j_, n_>(I2);
  RankFourTensor dR = f * RankFourTensor::IdentityFour() + g * d_W2_dW;
  if (theta >= small_theta)
  {
    const Real inv_2theta = 1.0 / (2.0 * theta);
    // (df/dθ · W_mn / (2θ)) · W_ij  → (df/dθ · inv_2θ) · W ⊗ W (output indices ij,mn).
    dR += (df_dth * inv_2theta) * W.template times<i_, j_, m_, n_>(W);
    dR += (dg_dth * inv_2theta) * W2.template times<i_, j_, m_, n_>(W);
  }
  // (For θ < small_theta the dθ-dependent contributions are O(θ) → 0; the
  //  identity + W² parts above cover the small-angle limit cleanly.)

  dR_dW = dR;
  return R;
}

void
LagrangianRashidRate::update(ComputeLagrangianObjectiveStress & host,
                             const RankTwoTensor & dS) const
{
  usingTensorIndices(i_, j_, k_, l_, m_, n_);
  const unsigned int qp = host._qp;

  // r̂ = exp(Δw) and its derivative.
  RankFourTensor d_rhat_d_dW;
  const RankTwoTensor rhat = rotationFromVorticity(host._vorticity_increment[qp],
                                                   d_rhat_d_dW);
  const RankTwoTensor rhatT = rhat.transpose();

  // σ_{n+1} = r̂ (σ_n + Δσ) r̂^T   (eq. 22)
  const RankTwoTensor S_inner = host._cauchy_stress_old[qp] + dS;
  host._cauchy_stress[qp] = rhat * S_inner * rhatT;

  // J^{-1}_{ijkl} = r̂_ik r̂_jl   (eq. 24), the rank-4 (r̂ ⊗ r̂) sandwich. Used for
  // the eigenstrain Jacobian and as the "outer" operator for the constitutive
  // piece. We name the dummy "middle pair" k, l here (output indices i, j, k, l)
  // — when chained via RankFourTensor::operator*, the k, l contract against
  // small_jacobian's first pair, as desired.
  const RankFourTensor J_inv = rhat.times<i_, k_, j_, l_>(rhat);

  // dσ/d(eigenstrain) = -J^{-1} : small_jacobian.
  // (mechanical_strain = total_strain - eigenstrain, hence the minus.)
  host._dcauchy_stress_d_eigenstrain[qp] = -(J_inv * host._small_jacobian[qp]);

  // Chain rule pieces for dσ/d(dL):
  //   d(Δw)/d(dL) = _d_vorticity_increment_d_F · inverse(_d_spatial_velocity_increment_d_F)
  //   d(Δd)/d(dL) = I^(4) − d(Δw)/d(dL)   (Δd + Δw = dL by construction).
  //   d(r̂)/d(dL)  = d(r̂)/d(Δw) · d(Δw)/d(dL).
  const RankFourTensor d_F_d_dL = host._d_spatial_velocity_increment_d_F[qp].inverse();
  const RankFourTensor d_dW_d_dL = host._d_vorticity_increment_d_F[qp] * d_F_d_dL;
  const RankFourTensor d_dD_d_dL = RankFourTensor::IdentityFour() - d_dW_d_dL;
  const RankFourTensor d_rhat_d_dL = d_rhat_d_dW * d_dW_d_dL;

  // Rotation pieces of dσ/d(dL):
  //   T1_{ijkl} = (d_rhat_d_dL)_{imkl} · (S r̂^T)_{mj}
  //   T2_{ijkl} = (r̂ S)_{im} · (d_rhat_d_dL)_{jmkl}
  // Implemented as RankTwoTensor.times<m, j, i, m, k, l>(RankFourTensor) contractions
  // (the templated contracted form: dummy index `m_` repeated).
  const RankTwoTensor SR = S_inner * rhatT;            // S r̂^T  (shape (m, j))
  const RankTwoTensor RS = rhat * S_inner;             // r̂ S    (shape (i, m))
  const RankFourTensor T1 = SR.times<m_, j_, i_, m_, k_, l_>(d_rhat_d_dL);
  const RankFourTensor T2 = RS.times<i_, m_, j_, m_, k_, l_>(d_rhat_d_dL);

  // Constitutive piece: J^{-1} : (small_jacobian · d(Δd)/d(dL))
  // = (r̂ ⊗ r̂)_{ijmn} (small_jacobian · d_dD_d_dL)_{mnkl}.
  const RankFourTensor T3 = J_inv * (host._small_jacobian[qp] * d_dD_d_dL);

  host._cauchy_jacobian[qp] = T1 + T2 + T3;
}

// ============================================================================
// Factory
// ============================================================================

std::unique_ptr<LagrangianObjectiveRate>
createObjectiveRate(const MooseEnum & rate_enum)
{
  const std::string s = rate_enum;
  if (s == "truesdell")
    return std::make_unique<LagrangianTruesdellRate>();
  if (s == "jaumann")
    return std::make_unique<LagrangianJaumannRate>();
  if (s == "green_naghdi")
    return std::make_unique<LagrangianGreenNaghdiRate>();
  if (s == "rashid")
    return std::make_unique<LagrangianRashidRate>();
  mooseError("Unknown objective_rate value: ", s);
}
