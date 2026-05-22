//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStrain.h"
#include "FactorizedRankTwoTensor.h"
#include "MathUtils.h"
#include "PermutationTensor.h"

template <class G>
InputParameters
ComputeLagrangianStrainBase<G>::baseParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("displacements", "Displacement variables");
  params.addParam<bool>(
      "large_kinematics", false, "Use large displacement kinematics in the kernel.");
  params.addParam<bool>("stabilize_strain", false, "Average the volumetric strains");
  params.addRangeCheckedParam<Real>(
      "alpha",
      1.0,
      "alpha >= 0.5 & alpha <= 1.0",
      "Generalized midpoint weight for the deformation gradient. 1.0 = backward Euler (default), "
      "0.5 = midpoint rule (matches Abaqus/Implicit).");
  MooseEnum kinematic_approximation(
      "linear quadratic rashid_approximate rashid_eigen", "linear");
  params.addParam<MooseEnum>(
      "kinematic_approximation",
      kinematic_approximation,
      "Approximation to the increment in the spatial velocity gradient: 'linear' (default; "
      "dL = I - f^{-1}), 'quadratic' (one more Taylor term), 'rashid_approximate' (Rashid's "
      "symmetric+skew formulas), or 'rashid_eigen' (exact log f via polar decomposition + "
      "matrix logs). Only affects large_kinematics; small kinematics is always linear.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", {}, "List of eigenstrains to account for");
  params.addParam<std::vector<MaterialPropertyName>>(
      "homogenization_gradient_names",
      {},
      "List of homogenization gradients to add to the displacement gradient");

  params.addParam<std::string>("base_name", "Material property base name");

  // We rely on this *not* having use_displaced mesh on
  params.suppressParameter<bool>("use_displaced_mesh");

  return params;
}

template <class G>
ComputeLagrangianStrainBase<G>::ComputeLagrangianStrainBase(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _grad_disp(coupledGradients("displacements")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _stabilize_strain(getParam<bool>("stabilize_strain")),
    _alpha(getParam<Real>("alpha")),
    _kinematic_approximation(
        getParam<MooseEnum>("kinematic_approximation").template getEnum<KinematicApproximation>()),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _eigenstrains_old(_eigenstrain_names.size()),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _rotated_mechanical_strain(
        declareProperty<RankTwoTensor>(_base_name + "rotated_mechanical_strain")),
    _rotated_mechanical_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "rotated_mechanical_strain")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _spatial_velocity_increment(
        declareProperty<RankTwoTensor>(_base_name + "spatial_velocity_increment")),
    _vorticity_increment(declareProperty<RankTwoTensor>(_base_name + "vorticity_increment")),
    _F_ust(declareProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_actual(declareProperty<RankTwoTensor>(_base_name + "actual_deformation_gradient")),
    _F_avg(declareProperty<RankTwoTensor>(_base_name + "average_deformation_gradient")),
    _F(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _f_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_incremental_deformation_gradient")),
    _d_spatial_velocity_increment_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_spatial_velocity_increment_d_deformation_gradient")),
    _d_vorticity_increment_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_vorticity_increment_d_deformation_gradient")),
    _d_F_d_grad_u(declareProperty<RankFourTensor>(
        _base_name + "d_deformation_gradient_d_grad_displacement")),
    _rotation(declareProperty<RankTwoTensor>(_base_name + "rotation")),
    _rotation_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "rotation")),
    _stretch(declareProperty<RankTwoTensor>(_base_name + "stretch")),
    _d_rotation_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_rotation_d_deformation_gradient")),
    _d_F_stab_d_F_ust(declareProperty<RankFourTensor>(
        _base_name + "d_F_stab_d_F_unstabilized")),
    _d_F_stab_d_F_avg(declareProperty<RankFourTensor>(
        _base_name + "d_F_stab_d_F_average")),
    _homogenization_gradient_names(
        getParam<std::vector<MaterialPropertyName>>("homogenization_gradient_names")),
    _homogenization_contributions(_homogenization_gradient_names.size()),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment"))
{
  // Couple old displacements only when the simulation is transient. With a Steady executioner
  // there is no "previous step", and the generalized midpoint rule treats the old state as
  // the undeformed reference (u_n = 0, grad u_n = 0, so F_n = I). The (1 - alpha) contribution
  // is then identically zero and we skip it in computeQpUnstabilizedDeformationGradient.
  if (_fe_problem.isTransient())
  {
    _disp_old = coupledValuesOld("displacements");
    _grad_disp_old = coupledGradientsOld("displacements");
  }

  // Setup eigenstrains
  for (auto i : make_range(_eigenstrain_names.size()))
  {
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
  }

  // In the future maybe there is a reason to have more than one, but for now
  if (_homogenization_gradient_names.size() > 1)
    mooseError("ComputeLagrangianStrainBase cannot accommodate more than one "
               "homogenization gradient");

  // Setup homogenization contributions
  for (unsigned int i = 0; i < _homogenization_gradient_names.size(); i++)
    _homogenization_contributions[i] =
        &getMaterialProperty<RankTwoTensor>(_homogenization_gradient_names[i]);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
  _mechanical_strain[_qp].zero();
  _rotated_mechanical_strain[_qp].zero();
  _F[_qp].setToIdentity();
  _rotation[_qp].setToIdentity();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeProperties()
{
  // Average the volumetric terms, if required
  computeDeformationGradient();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpProperties()
{
  // Add in the macroscale gradient contribution
  for (auto contribution : _homogenization_contributions)
    _F[_qp] += (*contribution)[_qp];

  // dF/d(grad u_{n+1}) = alpha * I^{(4)} for the generalized midpoint rule
  // (alpha = 1.0 reduces to backward Euler).
  _d_F_d_grad_u[_qp] = _alpha * RankFourTensor::IdentityFour();

  if (_large_kinematics)
  {
    _F_inv[_qp] = _F[_qp].inverse();
    _f_inv[_qp] = _F_old[_qp] * _F_inv[_qp];

    // Dispatch to the active kinematic-approximation helper. Each helper returns
    // dd (= Δd), dw (= Δw), and the f^{-1}-derivatives of dL = dd + dw and of dw alone.
    RankTwoTensor dd, dw;
    RankFourTensor d_dL_d_f_inv, d_dw_d_f_inv;
    computeQpLargeKinematicIncrement(_f_inv[_qp], dd, dw, d_dL_d_f_inv, d_dw_d_f_inv);

    // Common chain rule: d(f^{-1})_{pq}/dF_{mn} = -f^{-1}_{pm} * F^{-1}_{nq}.
    usingTensorIndices(p_, q_, m_, n_);
    const RankFourTensor d_f_inv_d_F =
        -_f_inv[_qp].template times<p_, m_, n_, q_>(_F_inv[_qp]);
    _d_spatial_velocity_increment_d_F[_qp] = d_dL_d_f_inv * d_f_inv_d_F;
    _d_vorticity_increment_d_F[_qp] = d_dw_d_f_inv * d_f_inv_d_F;

    setQpIncrementalStrains(dd, dw);
    computeQpPolarDecomposition();
  }
  // For small deformations we just provide the identity (and always linear)
  else
  {
    _F_inv[_qp] = RankTwoTensor::Identity();
    _f_inv[_qp] = RankTwoTensor::Identity();
    const RankTwoTensor dL = _F[_qp] - _F_old[_qp];

    // d(dL)/dF = I^{(4)} when dL = F - F_old. d(dW)/dF = the skew projector.
    _d_spatial_velocity_increment_d_F[_qp] = RankFourTensor::IdentityFour();
    {
      usingTensorIndices(i_, j_, k_, l_);
      const auto I2 = RankTwoTensor::Identity();
      _d_vorticity_increment_d_F[_qp] =
          0.5 * (RankFourTensor::IdentityFour() - I2.template times<j_, k_, i_, l_>(I2));
    }

    // Small kinematics: R = I, U = I, dR/dF = 0. Defensive defaults; GN is not used here.
    _rotation[_qp].setToIdentity();
    _stretch[_qp].setToIdentity();
    _d_rotation_d_F[_qp].zero();

    setQpIncrementalStrains(0.5 * (dL + dL.transpose()), 0.5 * (dL - dL.transpose()));
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpPolarDecomposition()
{
  // Polar decomposition F = R · U of the alpha-weighted, F-bar-stabilized deformation
  // gradient at this qp. We decompose _F rather than _F_actual because the rest of the
  // kernel/rate chain treats _F as the spatial frame; using _F here matches the pre-3.1
  // GN rate exactly (it read _def_grad, which is _F via ComputeLagrangianStressCauchy).
  const RankTwoTensor & F = _F[_qp];
  FactorizedRankTwoTensor C(F.transpose() * F);
  _stretch[_qp] = MathUtils::sqrt(C).get();
  const RankTwoTensor U_inv = MathUtils::sqrt(C).inverse().get();
  _rotation[_qp] = F * U_inv;

  // dR/dF closed form. See ComputeLagrangianObjectiveStress.C:221-227.
  const auto I = RankTwoTensor::Identity();
  const RankTwoTensor Y = _stretch[_qp].trace() * I - _stretch[_qp];
  const RankTwoTensor Z = _rotation[_qp] * Y;
  const RankTwoTensor O = Z * _rotation[_qp].transpose();
  usingTensorIndices(i_, j_, k_, l_);
  _d_rotation_d_F[_qp] =
      (O.template times<i_, k_, l_, j_>(Y) - Z.template times<i_, l_, k_, j_>(Z)) / Y.det();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpLargeKinematicIncrement(
    const RankTwoTensor & f_inv,
    RankTwoTensor & dd,
    RankTwoTensor & dw,
    RankFourTensor & d_dL_d_f_inv,
    RankFourTensor & d_dw_d_f_inv)
{
  switch (_kinematic_approximation)
  {
    case KinematicApproximation::Linear:
      computeLinearIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv);
      break;
    case KinematicApproximation::Quadratic:
      computeQuadraticIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv);
      break;
    case KinematicApproximation::RashidApproximate:
      computeRashidApproximateIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv);
      break;
    case KinematicApproximation::RashidEigen:
      computeRashidEigenIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv);
      break;
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpIncrementalStrains(const RankTwoTensor & dL)
{
  // Backward-compatible entry point: split into sym/skew and delegate.
  setQpIncrementalStrains(0.5 * (dL + dL.transpose()), 0.5 * (dL - dL.transpose()));
}

template <class G>
void
ComputeLagrangianStrainBase<G>::setQpIncrementalStrains(const RankTwoTensor & dd,
                                                        const RankTwoTensor & dw)
{
  _strain_increment[_qp] = dd;
  _vorticity_increment[_qp] = dw;
  // Full kinematic spatial velocity gradient increment, before any eigenstrain subtraction.
  // The objective-rate advection in ComputeLagrangianObjectiveStress consumes this.
  _spatial_velocity_increment[_qp] = dd + dw;

  // Increment the total strain
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  // Get rid of the eigenstrains
  // Note we currently do not alter the deformation gradient -- this will be
  // needed in the future for a "complete" system
  subtractQpEigenstrainIncrement(_strain_increment[_qp]);

  // Increment the mechanical strain
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

  // Additionally maintain the rotated mechanical-strain accumulator,
  // ε_n+1 = r̂ (ε_n + Δd) r̂^T, where r̂ = exp(Δw) via Rodrigues. This matches the
  // mechanical_strain output convention of `ComputeFiniteStrain` and is consumed only
  // by aux variables — the stress chain still uses the un-rotated `_mechanical_strain`
  // above. In small kinematics dw is small and r̂ ≈ I + dw + ½ dw², which contributes
  // only second-order corrections (equivalent to no rotation for small strain).
  RankTwoTensor r_hat;
  if (_large_kinematics)
  {
    const Real theta2 = 0.5 * dw.doubleContraction(dw);
    const Real theta = std::sqrt(theta2);
    Real f, g;
    const Real small_theta = 1.0e-7;
    if (theta < small_theta)
    {
      f = 1.0 - theta2 / 6.0;
      g = 0.5 - theta2 / 24.0;
    }
    else
    {
      f = std::sin(theta) / theta;
      g = (1.0 - std::cos(theta)) / theta2;
    }
    r_hat = RankTwoTensor::Identity() + f * dw + g * dw * dw;
  }
  else
  {
    r_hat = RankTwoTensor::Identity();
  }
  _rotated_mechanical_strain[_qp] =
      r_hat * (_rotated_mechanical_strain_old[_qp] + _strain_increment[_qp]) * r_hat.transpose();

  // Faked rotation increment for ComputeStressBase materials
  _rotation_increment[_qp] = RankTwoTensor::Identity();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeLinearIncrement(const RankTwoTensor & f_inv,
                                                       RankTwoTensor & dd,
                                                       RankTwoTensor & dw,
                                                       RankFourTensor & d_dL_d_f_inv,
                                                       RankFourTensor & d_dw_d_f_inv) const
{
  // Δl = I - f^{-1}, so Δd = sym(Δl), Δw = skew(Δl).
  const RankTwoTensor dL = RankTwoTensor::Identity() - f_inv;
  dd = 0.5 * (dL + dL.transpose());
  dw = 0.5 * (dL - dL.transpose());
  // d(Δl)/d(f^{-1}) = -I^{(4)}.
  d_dL_d_f_inv = -RankFourTensor::IdentityFour();
  // d(Δw)/d(f^{-1}) = - skew projector on f^{-1} = -(1/2)(I^{ikjl} - I^{iljk}).
  usingTensorIndices(i_, j_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();
  d_dw_d_f_inv =
      -0.5 * (RankFourTensor::IdentityFour() - I2.template times<j_, m_, i_, n_>(I2));
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQuadraticIncrement(const RankTwoTensor & f_inv,
                                                          RankTwoTensor & dd,
                                                          RankTwoTensor & dw,
                                                          RankFourTensor & d_dL_d_f_inv,
                                                          RankFourTensor & d_dw_d_f_inv) const
{
  // Δl = X + (1/2) X^2 with X = I - f^{-1} (one more Taylor term of -log f^{-1}).
  const RankTwoTensor X = RankTwoTensor::Identity() - f_inv;
  const RankTwoTensor dL = X + 0.5 * X * X;
  dd = 0.5 * (dL + dL.transpose());
  dw = 0.5 * (dL - dL.transpose());

  // dX/d(f^{-1}) = -I^{(4)}, and d(X^2)_{ij}/dX_{mn} = δ_{im} X_{nj} + X_{im} δ_{jn}.
  // So d(Δl)/d(f^{-1}) = -I^{(4)} - (1/2) (δ_{im} X_{nj} + X_{im} δ_{jn}).
  usingTensorIndices(i_, j_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();
  const RankFourTensor dXX_dX =
      I2.template times<i_, m_, n_, j_>(X) + X.template times<i_, m_, j_, n_>(I2);
  d_dL_d_f_inv = -RankFourTensor::IdentityFour() - 0.5 * dXX_dX;
  // dw = (1/2)(dL - dL^T), so d(dw)_{ij}/d... = (1/2)(d(dL)_{ij}/d... - d(dL)_{ji}/d...).
  d_dw_d_f_inv = 0.5 * (d_dL_d_f_inv - d_dL_d_f_inv.transposeIj());
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeRashidApproximateIncrement(
    const RankTwoTensor & f_inv,
    RankTwoTensor & dd,
    RankTwoTensor & dw,
    RankFourTensor & d_dL_d_f_inv,
    RankFourTensor & d_dw_d_f_inv) const
{
  // See plan_outline.pdf §2.3 (eq 10-15, with the corrected vorticity).
  // X = I - f^{-1}.  Symmetric part: A = X X^T - X - X^T,  Δd = -A/2 + A^2/4.
  // Skew part (from the rotation tensor): α_i = ε_ijk (f^{-1})_jk,
  //   cos θ = (tr(f^{-1}) - 1)/2, sin θ = √(1 - cos²θ),  Q = (1/4) α·α,
  //   Δw_ij = -(θ / (2√Q)) ε_ijk α_k.
  usingTensorIndices(i_, j_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();
  const auto I4 = RankFourTensor::IdentityFour();

  // ---- symmetric part ----
  const RankTwoTensor X = I2 - f_inv;
  const RankTwoTensor Xt = X.transpose();
  const RankTwoTensor A = X * Xt - X - Xt;
  dd = -0.5 * A + 0.25 * A * A;

  // dX/d(f^{-1}) = -I^{(4)}.
  // d(X X^T)_{ij}/dX_{mn} = δ_{im} X_{jn} + X_{in} δ_{jm}  (from (X X^T)_{ij} = X_{ik} X_{jk}).
  // d(X^T)_{ij}/dX_{mn} = δ_{jm} δ_{in}.
  // → d(A)/d(f^{-1}) = -[d(X X^T)/dX] + I^{(4)} + (transposed I^{(4)}).
  const RankFourTensor d_XXt_dX =
      I2.template times<i_, m_, j_, n_>(X) + X.template times<i_, n_, j_, m_>(I2);
  const RankFourTensor d_Xt_dX = I2.template times<j_, m_, i_, n_>(I2);
  const RankFourTensor dA_dfinv = -d_XXt_dX + I4 + d_Xt_dX;

  // d(A^2)_{ij}/dA_{mn} = δ_{im} A_{nj} + A_{im} δ_{jn}.
  const RankFourTensor d_AA_dA =
      I2.template times<i_, m_, n_, j_>(A) + A.template times<i_, m_, j_, n_>(I2);
  const RankFourTensor d_AA_dfinv = d_AA_dA * dA_dfinv;

  RankFourTensor d_dd_dfinv = -0.5 * dA_dfinv + 0.25 * d_AA_dfinv;

  // ---- skew part ----
  // α_i = ε_ijk (f^{-1})_jk  (axial vector of f^{-1}'s skew part, doubled).
  RealVectorValue alpha;
  for (unsigned int i = 0; i < 3; ++i)
  {
    Real ai = 0.0;
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        ai += PermutationTensor::eps(i, j, k) * f_inv(j, k);
    alpha(i) = ai;
  }
  // Derive θ from the axial vector's magnitude rather than from tr(f^{-1}): the trace
  // formula assumes f^{-1} is exactly a rotation, which is only true in the limit. Here
  // sin θ = √Q (Q = |skew part|²/4); a true rotation matches both, and an arbitrary f^{-1}
  // is projected onto its nearest "rotation-like" interpretation.
  const Real Q_raw = 0.25 * (alpha * alpha);
  // Clamp Q just below 1 so cos θ stays strictly positive (θ → π/2 is unphysical for one step).
  const Real Q = std::min(Q_raw, 1.0 - 1.0e-12);

  // Small-angle fallback: when sin θ → 0, θ/(2√Q) → 1/2 (L'Hopital) and Δw → -α/2,
  // which matches sym/skew of the linear approximation. d(Δw)/d(f^{-1}) is the
  // antisymmetrizer (1/2)(δ_im δ_jn - δ_jm δ_in).
  const Real small_Q = 1.0e-12;
  if (Q < small_Q)
  {
    // Δw_ij = -(1/2) ε_ijk α_k
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        Real v = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          v += PermutationTensor::eps(i, j, k) * alpha(k);
        dw(i, j) = -0.5 * v;
      }
    // d(Δw)/d(f^{-1}) = -(1/2)(δ_{im} δ_{jn} - δ_{jm} δ_{in})
    d_dw_d_f_inv = -0.5 * (I4 - I2.template times<j_, m_, i_, n_>(I2));
  }
  else
  {
    const Real sin_theta = std::sqrt(Q);
    const Real cos_theta = std::sqrt(1.0 - Q);
    const Real theta = std::asin(sin_theta);
    const Real coeff = -theta / (2.0 * sin_theta);

    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        Real v = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          v += PermutationTensor::eps(i, j, k) * alpha(k);
        dw(i, j) = coeff * v;
      }

    // Build d(Δw)/d(f^{-1}) analytically.
    //   Δw_ij = c(f^{-1}) * E_ij(f^{-1}),  E_ij = ε_ijk α_k,  c = -θ/(2 sin θ).
    //
    //   dα_k/d(f^{-1})_{mn} = ε_{kmn}  (Levi-Civita is constant).
    //   dQ/d(f^{-1})_{mn} = (1/2) α_k ε_{kmn}.
    //   With sin θ = √Q : dsin θ/d(f^{-1})_{mn} = α_k ε_{kmn} / (4 sin θ).
    //   dθ/d(f^{-1})_{mn} = dsin θ/d(f^{-1})_{mn} / cos θ.
    //   dc/dθ = -(sin θ - θ cos θ)/(2 sin² θ); dc/d(f^{-1})_{mn} = dc/dθ * dθ/d(f^{-1})_{mn}
    //     = (θ cos θ - sin θ) α_k ε_{kmn} / (8 sin³ θ cos θ).
    //   dE_ij/d(f^{-1})_{mn} = ε_{ijk} ε_{kmn}.
    //
    // Final:
    //   d(Δw)_{ij}/d(f^{-1})_{mn} = (dc/d(f^{-1})_{mn}) * E_ij + c * dE_ij/d(f^{-1})_{mn}.
    const Real dc_pref =
        (theta * cos_theta - sin_theta) / (8.0 * sin_theta * sin_theta * sin_theta * cos_theta);
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        Real E_ij = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          E_ij += PermutationTensor::eps(i, j, k) * alpha(k);
        for (unsigned int m = 0; m < 3; ++m)
          for (unsigned int n = 0; n < 3; ++n)
          {
            Real eps_alpha = 0.0;
            for (unsigned int k = 0; k < 3; ++k)
              eps_alpha += PermutationTensor::eps(k, m, n) * alpha(k);
            const Real dc_dfinv = dc_pref * eps_alpha;
            Real dE_dfinv = 0.0;
            for (unsigned int k = 0; k < 3; ++k)
              dE_dfinv +=
                  PermutationTensor::eps(i, j, k) * PermutationTensor::eps(k, m, n);
            d_dw_d_f_inv(i, j, m, n) = dc_dfinv * E_ij + coeff * dE_dfinv;
          }
      }
  }

  d_dL_d_f_inv = d_dd_dfinv + d_dw_d_f_inv;
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeRashidEigenIncrement(const RankTwoTensor & f_inv,
                                                            RankTwoTensor & dd,
                                                            RankTwoTensor & dw,
                                                            RankFourTensor & d_dL_d_f_inv,
                                                            RankFourTensor & d_dw_d_f_inv) const
{
  // See plan_outline.pdf §2.4. Polar-decompose f^{-1} = r' u', with u' symmetric positive
  // definite and r' a proper rotation. The PDF's identity `log f^{-1} = -log d - log w`
  // only holds when log u and log r commute (i.e. for non-rotating deformation); in general
  // the right stretch of f^{-1} is u' = R · U^{-1} · R^T where R, U are the right polar of f.
  // So -log(u') = R · log U · R^T is the *spatial-frame* log strain, not the co-rotated
  // log U. We compute it that way first, then rotate by r' = R^T to land in the n-frame
  // so that _strain_increment = log U exactly. This is the strain measure the Rashid
  // objective stress update consumes (eq. 22, σ_{n+1} = r̂ (σ_n + Δσ) r̂^T expects Δσ
  // in the n-frame).
  usingTensorIndices(a_, b_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();

  // ---- polar decomposition of f^{-1} ----
  FactorizedRankTwoTensor cprime(f_inv.transpose() * f_inv);
  const RankTwoTensor u = MathUtils::sqrt(cprime).get();
  const RankTwoTensor u_inv = MathUtils::sqrt(cprime).inverse().get();
  const RankTwoTensor r = f_inv * u_inv;

  // ---- intermediate Δd_spatial = -log u' = R · log U · R^T (n+1 frame) ----
  FactorizedRankTwoTensor uFact(u);
  const RankTwoTensor log_u = MathUtils::log(uFact).get();
  const RankTwoTensor dd_spatial = -log_u;

  // d(log u)/d(c') = (1/2) dlog(c'), and d(c')_{ab}/d(f^{-1})_{mn} = δ_{an} f_inv_{mb}
  //                                                                + δ_{bn} f_inv_{ma}.
  const RankFourTensor dlog_cprime = MathUtils::dlog(cprime);
  const RankFourTensor d_cprime_d_finv =
      I2.template times<a_, n_, m_, b_>(f_inv) + I2.template times<b_, n_, m_, a_>(f_inv);
  const RankFourTensor d_dd_spatial_d_finv = -0.5 * (dlog_cprime * d_cprime_d_finv);

  // ---- d(r)/d(f^{-1}) via the polar-decomposition closed form (same trick as
  //      ComputeLagrangianObjectiveStress::polarDecomposition). ----
  const RankTwoTensor Y = u.trace() * I2 - u;
  const RankTwoTensor Z = r * Y;
  const RankTwoTensor O = Z * r.transpose();
  usingTensorIndices(i_, j_, k_, l_);
  const RankFourTensor d_r_d_finv =
      (O.template times<i_, k_, l_, j_>(Y) - Z.template times<i_, l_, k_, j_>(Z)) / Y.det();

  // ---- Δw = -log r via Rodrigues. ----
  // log r = φ(θ) (r - r^T) with φ(θ) = θ/(2 sin θ),  cos θ = (tr r - 1)/2.
  const Real cos_theta = MathUtils::clamp(0.5 * (r.trace() - 1.0), -1.0, 1.0);
  const Real sin2 = std::max(1.0 - cos_theta * cos_theta, 0.0);
  const Real sin_theta = std::sqrt(sin2);
  const Real theta = std::acos(cos_theta);
  const RankTwoTensor A = r - r.transpose();

  // d(log r)_{ij}/d(r)_{mn} = (dφ/dr_{mn}) A_{ij} + φ (δ_{im} δ_{jn} - δ_{jm} δ_{in}).
  //   dφ/dr_{mn} = (dφ/dθ)(dθ/d cos θ)(d cos θ/dr_{mn})
  //              = ψ δ_{mn}, where ψ = (θ cos θ - sin θ)/(4 sin³ θ).
  // Small-angle: φ → 1/2, ψ → -1/12, so d(log r)/dr → (1/2)(I^(4) - swap_ij).
  const Real small_sin = 1.0e-7;
  RankFourTensor d_logr_d_r;
  RankTwoTensor log_r;
  const RankFourTensor swap_ij = I2.template times<j_, m_, i_, n_>(I2);
  if (std::abs(sin_theta) < small_sin)
  {
    log_r = 0.5 * A;
    d_logr_d_r = 0.5 * (RankFourTensor::IdentityFour() - swap_ij);
  }
  else
  {
    const Real phi = theta / (2.0 * sin_theta);
    const Real psi = (theta * cos_theta - sin_theta) / (4.0 * sin_theta * sin2);
    log_r = phi * A;
    // (dφ/dr)_{mn} = ψ δ_{mn} → outer with A_{ij} gives A.times<i_, j_, m_, n_>(I2) * ψ.
    const RankFourTensor dphi_outer_A = A.template times<i_, j_, m_, n_>(I2);
    d_logr_d_r = psi * dphi_outer_A + phi * (RankFourTensor::IdentityFour() - swap_ij);
  }
  dw = -log_r;
  d_dw_d_f_inv = -(d_logr_d_r * d_r_d_finv);

  // ---- Rotate Δd from spatial back to co-rotated (n) frame: log U = r' · (R log U R^T) · r'^T,
  //      using r' = R^T from the polar of f^{-1}. The derivative of the sandwich r'·A·r'^T
  //      w.r.t. f^{-1} expands to three rank-4 pieces (chain rule on r' AND on A). The
  //      6-argument `times<>(RankFourTensor)` overload uses x[0..4] (one dummy at index 4),
  //      so we reuse the same dummy label `p2_` in each sub-contraction.
  {
    usingTensorIndices(i2_, j2_, m2_, n2_, p2_);
    dd = r * dd_spatial * r.transpose();
    const RankTwoTensor M = dd_spatial * r.transpose();  // dd · r^T   (p, j) shape
    const RankTwoTensor N = r * dd_spatial;              // r · dd     (i, q) shape
    // T1_{ijmn} = (dr/d_finv)_{ip,mn} · M_{pj}
    const RankFourTensor T1 =
        M.template times<p2_, j2_, i2_, p2_, m2_, n2_>(d_r_d_finv);
    // T2_{ijmn} = r_{ip} · (d_dd_spatial_d_finv)_{pq,mn} · r_{jq}: contract twice over the dummy.
    const RankFourTensor mid =
        r.template times<i2_, p2_, p2_, j2_, m2_, n2_>(d_dd_spatial_d_finv);
    const RankFourTensor T2 =
        r.template times<j2_, p2_, i2_, p2_, m2_, n2_>(mid);
    // T3_{ijmn} = N_{iq} · (dr/d_finv)_{jq,mn}
    const RankFourTensor T3 =
        N.template times<i2_, p2_, j2_, p2_, m2_, n2_>(d_r_d_finv);
    const RankFourTensor d_dd_d_finv = T1 + T2 + T3;
    d_dL_d_f_inv = d_dd_d_finv + d_dw_d_f_inv;
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::subtractQpEigenstrainIncrement(RankTwoTensor & strain)
{
  for (auto i : make_range(_eigenstrain_names.size()))
    strain -= (*_eigenstrains[i])[_qp] - (*_eigenstrains_old[i])[_qp];
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpUnstabilizedDeformationGradient()
{
  // Generalized midpoint: F^alpha_{n+1} = I + alpha * (grad u_{n+1}, u_{n+1})
  //                                         + (1 - alpha) * (grad u_n, u_n).
  // alpha = 1.0 reduces to backward Euler (no old contribution). With a Steady executioner
  // the old displacement is treated as identically zero (F_n = I), so we skip the old call
  // entirely - this lets a user run with alpha != 1 in steady mode as well.
  _F_ust[_qp].setToIdentity();
  const bool include_old = _alpha != 1.0 && _fe_problem.isTransient();
  for (auto component : make_range(_ndisp))
  {
    G::addGradOp(_F_ust[_qp],
                 component,
                 _alpha * (*_grad_disp[component])[_qp],
                 _alpha * (*_disp[component])[_qp],
                 _q_point[_qp]);
    if (include_old)
      G::addGradOp(_F_ust[_qp],
                   component,
                   (1.0 - _alpha) * (*_grad_disp_old[component])[_qp],
                   (1.0 - _alpha) * (*_disp_old[component])[_qp],
                   _q_point[_qp]);
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpActualDeformationGradient()
{
  // The literal deformation gradient at n+1 (no alpha weighting, no F-bar). For alpha = 1
  // this is identical to _F_ust.
  _F_actual[_qp].setToIdentity();
  for (auto component : make_range(_ndisp))
    G::addGradOp(_F_actual[_qp],
                 component,
                 (*_grad_disp[component])[_qp],
                 (*_disp[component])[_qp],
                 _q_point[_qp]);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeDeformationGradient()
{
  // First calculate the unstabilized deformation gradient at each qp
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    computeQpUnstabilizedDeformationGradient();
    computeQpActualDeformationGradient();
    _F[_qp] = _F_ust[_qp];
  }

  usingTensorIndices(i_, j_, k_, l_);
  const auto I2 = RankTwoTensor::Identity();

  // If stabilization is on do the volumetric correction
  if (_stabilize_strain)
  {
    const auto F_avg = StabilizationUtils::elementAverage(
        [this](unsigned int qp) { return _F_ust[qp]; }, _JxW, _coord);
    // All quadrature points have the same F_avg
    _F_avg.set().setAllValues(F_avg);
    // Make the appropriate modification, depending on small or large
    // deformations
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (_large_kinematics)
      {
        // Multiplicative F-bar: F_stab = (det F_avg / det F_ust)^{1/3} * F_ust.
        // Chain-rule partials:
        //   dF_stab/dF_ust = gamma * I^(4) - (gamma/3) * F_ust ⊗ F_ust^{-T}
        //   dF_stab/dF_avg = (gamma/3) * F_ust ⊗ F_avg^{-T}
        const Real gamma = std::pow(F_avg.det() / _F[_qp].det(), 1.0 / 3.0);
        const auto Fust_invT = _F_ust[_qp].inverse().transpose();
        const auto Favg_invT = F_avg.inverse().transpose();
        _d_F_stab_d_F_ust[_qp] = gamma * RankFourTensor::IdentityFour() -
                                 (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(Fust_invT);
        _d_F_stab_d_F_avg[_qp] =
            (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(Favg_invT);
        _F[_qp] *= gamma;
      }
      else
      {
        // Additive (trace) F-bar: F_stab = F_ust + (tr(F_avg - F_ust)/3) * I.
        // dF_stab/dF_ust = I^(4) - (1/3) * I2 ⊗ I2  (each diagonal component pulled out).
        // dF_stab/dF_avg = (1/3) * I2 ⊗ I2.
        const auto outer = I2.template times<i_, j_, k_, l_>(I2);
        _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour() - (1.0 / 3.0) * outer;
        _d_F_stab_d_F_avg[_qp] = (1.0 / 3.0) * outer;
        _F[_qp] += (F_avg.trace() - _F[_qp].trace()) * I2 / 3.0;
      }
    }
  }
  else
  {
    // F-bar off: dF_stab/dF_ust = I^(4), dF_stab/dF_avg = 0.
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour();
      _d_F_stab_d_F_avg[_qp].zero();
    }
  }
}

template class ComputeLagrangianStrainBase<GradientOperatorCartesian>;
template class ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>;
template class ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>;
