//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangianObjectiveRate.h"
#include "MooseEnum.h"
#include "MooseError.h"

#include <tuple>

// ============================================================================
// File-local helpers (formerly static members of the rate class hierarchy)
// ============================================================================
namespace
{
/// Build the J tensor that defines the linear advection `sigma_{n+1} = J(dQ)^{-1} sigma`.
RankFourTensor
updateTensor(const RankTwoTensor & dQ)
{
  const auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return (1.0 + dQ.trace()) * I.times<i, k, j, l>(I) - dQ.times<i, k, j, l>(I) -
         I.times<i, k, j, l>(dQ);
}

/// Apply the linear advection to a stress, returning `(advected_stress, Jinv)`.
std::tuple<RankTwoTensor, RankFourTensor>
advectStress(const RankTwoTensor & S0, const RankTwoTensor & dQ)
{
  const RankFourTensor J = updateTensor(dQ);
  const RankFourTensor Jinv = J.inverse();
  const RankTwoTensor S = Jinv * S0;
  return {S, Jinv};
}

/// Residual-only advection: solve `J(dQ) : S = S0` for the single stress RHS instead of forming
/// the full fourth-order inverse. `Jinv` is only needed as an operator on the Jacobian, so on
/// residual-only sweeps a one-RHS 9x9 solve replaces the 9-column inverse (~2x fewer flops on the
/// most frequently evaluated path). Correct for any dQ: `J` maps the symmetric subspace to itself,
/// so the solution equals `Jinv * S0` and stays symmetric even when dQ is not.
RankTwoTensor
advectStressSolve(const RankTwoTensor & S0, const RankTwoTensor & dQ)
{
  const RankFourTensor J = updateTensor(dQ);
  // Row/col index (a, b) -> (3a + b); A_{(ij)(kl)} = J_ijkl matches RankFourTensor::operator*.
  Eigen::Matrix<Real, 9, 9> A;
  Eigen::Matrix<Real, 9, 1> b;
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
    {
      b(3 * i + j) = S0(i, j);
      for (const auto k : make_range(3u))
        for (const auto l : make_range(3u))
          A(3 * i + j, 3 * k + l) = J(i, j, k, l);
    }
  const Eigen::Matrix<Real, 9, 1> x = A.partialPivLu().solve(b);
  RankTwoTensor S;
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      S(i, j) = x(3 * i + j);
  return S;
}

/// Derivative of the linear advection action with respect to the kinematic tensor.
RankFourTensor
stressAdvectionDerivative(const RankTwoTensor & S)
{
  const auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return S.times<i, j, k, l>(I) - I.times<i, k, l, j>(S) - S.times<i, l, j, k>(I);
}

/// Consistent tangent for the linear template.
RankFourTensor
cauchyJacobian(const RankFourTensor & Jinv,
               const RankFourTensor & small_jacobian,
               const RankFourTensor & U)
{
  return Jinv * (small_jacobian - U);
}

/// Rodrigues exponential of a skew 3x3 tensor W. When `dR_dW` is non-null it is filled with the
/// unconstrained chain-rule derivative; otherwise the R4 algebra is skipped. The upstream
/// `d(dW)/dF` projects out non-skew perturbations.
RankTwoTensor
rotationFromVorticity(const RankTwoTensor & W, RankFourTensor * dR_dW)
{
  // For a skew W in 3D, theta = sqrt((W : W) / 2). R = exp(W) via Rodrigues:
  //   R = I + f(theta) W + g(theta) W^2,  f = sin theta / theta,  g = (1 - cos theta) / theta^2.
  // For theta -> 0, fall back to the Taylor expansion: R ~= I + W + W^2/2.
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

  if (!dR_dW)
    return R;

  // dR_ij/dW_mn = (df/dtheta * dtheta/dW_mn) W_ij + f * delta_im delta_jn
  //             + (dg/dtheta * dtheta/dW_mn) (W^2)_ij + g * (delta_im W_nj + W_im delta_jn)
  // dtheta/dW_mn = W_mn / (2theta)   (from d(theta^2)/dW = W).
  usingTensorIndices(i_, j_, m_, n_);
  const RankFourTensor d_W2_dW =
      I2.template times<i_, m_, n_, j_>(W) + W.template times<i_, m_, j_, n_>(I2);
  *dR_dW = f * RankFourTensor::IdentityFour() + g * d_W2_dW;
  if (theta >= small_theta)
  {
    const Real inv_2theta = 1.0 / (2.0 * theta);
    // (df/dtheta * W_mn / (2theta)) * W_ij  -> (df/dtheta * inv_2theta) * W (x) W (output indices
    // ij,mn).
    *dR_dW += (df_dth * inv_2theta) * W.template times<i_, j_, m_, n_>(W);
    *dR_dW += (dg_dth * inv_2theta) * W2.template times<i_, j_, m_, n_>(W);
  }
  // (For theta < small_theta the dtheta-dependent contributions are O(theta) -> 0; the
  //  identity + W^2 parts above cover the small-angle limit cleanly.)

  return R;
}
}

namespace LagrangianObjectiveRates
{

// ============================================================================
// Truesdell
// ============================================================================
Outputs
truesdell(const Inputs & in, bool need_jacobian)
{
  Outputs out;
  if (!need_jacobian)
  {
    out.cauchy_stress = advectStressSolve(in.cauchy_stress_old + in.dS, in.dL);
    return out;
  }

  auto [S, Jinv] = advectStress(in.cauchy_stress_old + in.dS, in.dL);
  out.cauchy_stress = S;
  out.dcauchy_stress_d_eigenstrain = -Jinv * in.small_jacobian;

  const RankFourTensor U = stressAdvectionDerivative(S);
  out.cauchy_jacobian = cauchyJacobian(Jinv, in.small_jacobian, U);
  return out;
}

// ============================================================================
// Jaumann
// ============================================================================
Outputs
jaumann(const Inputs & in, bool need_jacobian)
{
  Outputs out;
  if (!need_jacobian)
  {
    out.cauchy_stress = advectStressSolve(in.cauchy_stress_old + in.dS, in.dW);
    return out;
  }

  auto [S, Jinv] = advectStress(in.cauchy_stress_old + in.dS, in.dW);
  out.cauchy_stress = S;
  out.dcauchy_stress_d_eigenstrain = -Jinv * in.small_jacobian;

  const RankFourTensor d_dW_d_dL = in.d_dW_d_F * in.d_dL_d_F.inverse();
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dW_d_dL;
  out.cauchy_jacobian = cauchyJacobian(Jinv, in.small_jacobian, U);
  return out;
}

// ============================================================================
// Green-Naghdi
// ============================================================================
Outputs
greenNaghdi(const Inputs & in, bool need_jacobian)
{
  usingTensorIndices(i, j, k, l, m);

  const RankTwoTensor I = RankTwoTensor::Identity();
  const RankTwoTensor dR = in.rotation * in.rotation_old.transpose() - I;
  const RankTwoTensor dO = dR * in.inv_df;

  Outputs out;
  if (!need_jacobian)
  {
    out.cauchy_stress = advectStressSolve(in.cauchy_stress_old + in.dS, dO);
    return out;
  }

  auto [S, Jinv] = advectStress(in.cauchy_stress_old + in.dS, dO);
  out.cauchy_stress = S;
  out.dcauchy_stress_d_eigenstrain = -Jinv * in.small_jacobian;

  const RankFourTensor & d_R_d_F = in.d_rotation_d_F;
  const RankFourTensor d_F_d_dL = in.d_dL_d_F.inverse();
  const RankTwoTensor T = in.rotation_old.transpose() * in.inv_df;

  const RankFourTensor d_invdf_d_F = -in.inv_df.times<i, k, l, j>(in.inv_def_grad);
  const RankFourTensor d_invdf_d_dL = d_invdf_d_F * d_F_d_dL;
  const RankFourTensor d_dO_d_invdf = dR.times<i, k, j, l>(I);

  const RankFourTensor d_dO_d_dL =
      T.times<m, j, i, m, k, l>(d_R_d_F * d_F_d_dL) + d_dO_d_invdf * d_invdf_d_dL;
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dO_d_dL;
  out.cauchy_jacobian = cauchyJacobian(Jinv, in.small_jacobian, U);
  return out;
}

// ============================================================================
// Rashid
// ============================================================================
Outputs
rashid(const Inputs & in, bool need_jacobian)
{
  usingTensorIndices(i_, j_, k_, l_, m_, n_);

  // r_hat = exp(Deltaw). Skip its R4 derivative when we don't need the Jacobian.
  RankFourTensor d_rhat_d_dW;
  const RankTwoTensor rhat = rotationFromVorticity(in.dW, need_jacobian ? &d_rhat_d_dW : nullptr);
  const RankTwoTensor rhatT = rhat.transpose();

  // sigma_{n+1} = r_hat (sigma_n + Deltasigma) r_hat^T   (eq. 22)
  const RankTwoTensor S_inner = in.cauchy_stress_old + in.dS;
  Outputs out;
  out.cauchy_stress = rhat * S_inner * rhatT;

  if (!need_jacobian)
    return out;

  // J^{-1}_{ijkl} = r_hat_ik r_hat_jl   (eq. 24), the rank-4 (r_hat (x) r_hat) sandwich. Used for
  // the eigenstrain Jacobian and as the "outer" operator for the constitutive piece. We name the
  // dummy "middle pair" k, l here (output indices i, j, k, l) -- when chained via
  // RankFourTensor::operator*, the k, l contract against small_jacobian's first pair, as desired.
  const RankFourTensor J_inv = rhat.times<i_, k_, j_, l_>(rhat);

  // dsigma/d(eigenstrain) = -J^{-1} : small_jacobian.
  // (mechanical_strain = total_strain - eigenstrain, hence the minus.)
  out.dcauchy_stress_d_eigenstrain = -(J_inv * in.small_jacobian);

  // Chain rule pieces for dsigma/d(dL):
  //   d(Deltaw)/d(dL) = d(dW)/dF * inverse(d(dL)/dF)
  //   d(Deltad)/d(dL) = I^(4) - d(Deltaw)/d(dL)   (Deltad + Deltaw = dL by construction).
  //   d(r_hat)/d(dL)  = d(r_hat)/d(Deltaw) * d(Deltaw)/d(dL).
  const RankFourTensor d_F_d_dL = in.d_dL_d_F.inverse();
  const RankFourTensor d_dW_d_dL = in.d_dW_d_F * d_F_d_dL;
  const RankFourTensor d_dD_d_dL = RankFourTensor::IdentityFour() - d_dW_d_dL;
  const RankFourTensor d_rhat_d_dL = d_rhat_d_dW * d_dW_d_dL;

  // Rotation pieces of dsigma/d(dL):
  //   T1_{ijkl} = (d_rhat_d_dL)_{imkl} * (S r_hat^T)_{mj}
  //   T2_{ijkl} = (r_hat S)_{im} * (d_rhat_d_dL)_{jmkl}
  const RankTwoTensor SR = S_inner * rhatT; // S r_hat^T  (shape (m, j))
  const RankTwoTensor RS = rhat * S_inner;  // r_hat S    (shape (i, m))
  const RankFourTensor T1 = SR.times<m_, j_, i_, m_, k_, l_>(d_rhat_d_dL);
  const RankFourTensor T2 = RS.times<i_, m_, j_, m_, k_, l_>(d_rhat_d_dL);

  // Constitutive piece: J^{-1} : (small_jacobian * d(Deltad)/d(dL)).
  const RankFourTensor T3 = J_inv * (in.small_jacobian * d_dD_d_dL);

  out.cauchy_jacobian = T1 + T2 + T3;
  return out;
}

// ============================================================================
// Dispatch
// ============================================================================
Outputs
compute(const MooseEnum & rate, const Inputs & in, bool need_jacobian)
{
  const std::string s = rate;
  if (s == "truesdell")
    return truesdell(in, need_jacobian);
  if (s == "jaumann")
    return jaumann(in, need_jacobian);
  if (s == "green_naghdi")
    return greenNaghdi(in, need_jacobian);
  if (s == "rashid")
    return rashid(in, need_jacobian);
  mooseError("Unknown objective_rate value: ", s);
}
}
