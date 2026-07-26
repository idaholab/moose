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
  MooseEnum F_bar_mode("total incremental", "total");
  params.addParam<MooseEnum>(
      "F_bar_mode",
      F_bar_mode,
      "What deformation gradient F-bar averages over (only used when `stabilize_strain = true`). "
      "'total' (default) averages the full F at each qp and rescales each qp's F by "
      "cbrt(det(F_avg)/det(F_ust)). 'incremental' averages the incremental F "
      "(F_ust * F_ust_old^{-1}) at each qp and rescales by cbrt(det(f_avg)/det(f_ust)); this is "
      "bit-for-bit compatible with the OLD `ComputeFiniteStrain` + `volumetric_locking_correction "
      "= "
      "true` formulation. Set to 'incremental' when cross-checking against the old kernel system.");
  params.addParam<bool>(
      "publish_rotation_increment",
      false,
      "If true, publish `rotation_increment = exp(vorticity_increment)` (Rodrigues) for "
      "downstream consumers that rotate by it (e.g. `ComputeMultiPlasticityStress` with "
      "`perform_finite_strain_rotations = true`). Default false keeps `rotation_increment = I` "
      "(the historical behavior -- the Lagrangian objective-rate machinery applies rotation "
      "externally). Enable when wrapping plasticity that needs its internal stress state to "
      "track the rotated Cauchy stress between steps, in tandem with `rotate_old_stress = true` "
      "on the objective rate.");
  params.addRangeCheckedParam<Real>(
      "alpha",
      1.0,
      "alpha >= 0.5 & alpha <= 1.0",
      "Generalized midpoint weight for the deformation gradient. 1.0 = backward Euler (default), "
      "0.5 = midpoint rule (matches Abaqus/Implicit).");
  MooseEnum kinematic_approximation("linear quadratic rashid_approximate rashid_eigen", "linear");
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
    GuaranteeProvider(this),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _grad_disp(coupledGradients("displacements")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _stabilize_strain(getParam<bool>("stabilize_strain")),
    _F_bar_mode(getParam<MooseEnum>("F_bar_mode").template getEnum<FBarMode>()),
    _publish_rotation_increment(getParam<bool>("publish_rotation_increment")),
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
    _deformation_gradient_increment(
        declareProperty<RankTwoTensor>(_base_name + "spatial_deformation_gradient_increment")),
    _vorticity_increment(declareProperty<RankTwoTensor>(_base_name + "vorticity_increment")),
    _F_ust(declareProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_ust_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_actual(declareProperty<RankTwoTensor>(_base_name + "actual_deformation_gradient")),
    _F_avg(declareProperty<RankTwoTensor>(_base_name + "average_deformation_gradient")),
    _F(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _f_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_incremental_deformation_gradient")),
    _F_ust_inv(
        declareProperty<RankTwoTensor>(_base_name + "inverse_unstabilized_deformation_gradient")),
    _F_ust_det(declareProperty<Real>(_base_name + "det_unstabilized_deformation_gradient")),
    _d_deformation_gradient_increment_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_spatial_deformation_gradient_increment_d_deformation_gradient")),
    _d_vorticity_increment_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_vorticity_increment_d_deformation_gradient")),
    _d_F_d_grad_u(
        declareProperty<RankFourTensor>(_base_name + "d_deformation_gradient_d_grad_displacement")),
    _rotation(declareProperty<RankTwoTensor>(_base_name + "rotation")),
    _stretch(declareProperty<RankTwoTensor>(_base_name + "stretch")),
    _d_rotation_d_F(
        declareProperty<RankFourTensor>(_base_name + "d_rotation_d_deformation_gradient")),
    _d_F_stab_d_F_ust(declareProperty<RankFourTensor>(_base_name + "d_F_stab_d_F_unstabilized")),
    _d_F_stab_d_F_avg(declareProperty<RankFourTensor>(_base_name + "d_F_stab_d_F_average")),
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

  // The strain calculator is the single source of truth for the kinematics regime. Publish a
  // LARGE_KINEMATICS guarantee on the deformation gradient (issued in the constructor so it is in
  // place before any consumer's initialSetup); the Lagrangian stress calculators and
  // stress-divergence kernels derive their own `large_kinematics` from it. Small kinematics leaves
  // the guarantee absent, which the consumers read as `large_kinematics = false`.
  if (_large_kinematics)
    issueGuarantee(_base_name + "deformation_gradient", Guarantee::LARGE_KINEMATICS);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
  _mechanical_strain[_qp].zero();
  _rotated_mechanical_strain[_qp].zero();
  _F[_qp].setToIdentity();
  _F_ust[_qp].setToIdentity();
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
  // Add in the macroscale gradient contribution to both the stabilized `_F` (used by the
  // strain chain via `_f_inv`) AND the unstabilized `_F_ust` (used by the new F_ust-wrap
  // architecture in the stress materials). The homogenization gradient is a real
  // deformation imposed via the scalar constraint, not a stabilization -- it must appear
  // in F_ust too or PK1 = det(F_ust) sigma F_ust^{-T} will be missing its contribution.
  for (auto contribution : _homogenization_contributions)
  {
    _F[_qp] += (*contribution)[_qp];
    _F_ust[_qp] += (*contribution)[_qp];
  }

  // Publish F_ust^{-1} and det(F_ust) for the large-kinematics consumers' spatial push-forward
  // (grad_x = F_ust^{-T} grad_X, J_ust = det F_ust) and the Cauchy -> PK1 wrap. Computed once per
  // qp here -- shared by all displacement kernels and the stress calculator via the material
  // system -- instead of recomputed per test/trial/qp downstream. Gated on `_large_kinematics`
  // (every consumer reads these only on the large-kinematics path) and on `isPropertyActive` (so
  // consumers that never request them pay nothing).
  if (_large_kinematics && isPropertyActive(_F_ust_inv.id()))
  {
    _F_ust_inv[_qp] = _F_ust[_qp].inverse();
    _F_ust_det[_qp] = _F_ust[_qp].det();
  }

  // Skip the Jacobian-only RankFourTensor derivative chain when no downstream consumer
  // will read it (i.e. we're in a residual-only sweep). All of `_d_F_d_grad_u`,
  // `_d_deformation_gradient_increment_d_F`, `_d_vorticity_increment_d_F`, and the
  // `_d_rotation_d_F` slot of the polar decomposition feed only `*_jacobian` material
  // properties, which the kernel consumes only during Jacobian or
  // residual-and-Jacobian-together assembly.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();

  // dF/d(grad u_{n+1}) = alpha * I^{(4)} for the generalized midpoint rule
  // (alpha = 1.0 reduces to backward Euler).
  if (need_jacobian)
    _d_F_d_grad_u[_qp] = _alpha * RankFourTensor::IdentityFour();

  if (_large_kinematics)
  {
    _F_inv[_qp] = _F[_qp].inverse();
    // For `F_bar_mode = incremental`: `_f_inv` must invert the *incremental* F that was
    // F-bar'd (= `gamma_inc * F_ust * F_ust_old^{-1}`, matching OLD `ComputeFiniteStrain`'s
    // `_Fhat` after its volumetric-locking correction). Since `_F = gamma_inc * F_ust`,
    // `(gamma_inc * F_ust * F_ust_old^{-1})^{-1} = F_ust_old * _F^{-1}` -- i.e., pair the
    // unstabilized old F with the (cumulative) stabilized current `_F^{-1}`. Using
    // `_F_old` (the *cumulative* F-bar'd previous-step `_F`) here would compound F-bar
    // across steps and break OLD-compat. `total` mode keeps the existing form (where `_F`
    // and `_F_old` are the cumulative full-F F-bar'd values and the ratio gives the
    // F-bar'd incremental F directly).
    if (_stabilize_strain && _F_bar_mode == FBarMode::Incremental)
      _f_inv[_qp] = _F_ust_old[_qp] * _F_inv[_qp];
    else
      _f_inv[_qp] = _F_old[_qp] * _F_inv[_qp];

    // Dispatch to the active kinematic-approximation helper. Each helper returns
    // dd (= Deltad), dw (= Deltaw), and -- only when `need_jacobian` -- the
    // f^{-1}-derivatives of dL = dd + dw and of dw alone. The (dd, dw) outputs are needed
    // every iteration; the RankFour derivatives feed only the Jacobian chain below, so on
    // residual-only sweeps the helper skips them entirely.
    RankTwoTensor dd, dw;
    RankFourTensor d_dL_d_f_inv, d_dw_d_f_inv;
    computeQpLargeKinematicIncrement(
        _f_inv[_qp], dd, dw, d_dL_d_f_inv, d_dw_d_f_inv, need_jacobian);

    if (need_jacobian)
    {
      // Common chain rule: d(f^{-1})_{pq}/dF_{mn} = -f^{-1}_{pm} * F^{-1}_{nq}.
      usingTensorIndices(p_, q_, m_, n_);
      const RankFourTensor d_f_inv_d_F = -_f_inv[_qp].template times<p_, m_, n_, q_>(_F_inv[_qp]);
      _d_deformation_gradient_increment_d_F[_qp] = d_dL_d_f_inv * d_f_inv_d_F;
      _d_vorticity_increment_d_F[_qp] = d_dw_d_f_inv * d_f_inv_d_F;
    }

    setQpIncrementalStrains(dd, dw);
    // The polar decomposition (a tensor eigensolve + sqrt per qp, on every residual and Jacobian
    // eval) feeds only the Green-Naghdi objective rate's `_rotation` / `_d_rotation_d_F` (and its
    // internal `_stretch`). Skip it entirely when no active consumer needs it -- Truesdell uses
    // the deformation-gradient increment, Jaumann the vorticity increment, Rashid the Rodrigues
    // `_rotation_increment`. The active-property set self-corrects for future consumers with no
    // coupling flag.
    if (isPropertyActive(_rotation.id()) || isPropertyActive(_stretch.id()))
      computeQpPolarDecomposition(need_jacobian);
  }
  // For small deformations we just provide the identity (and always linear)
  else
  {
    _F_inv[_qp] = RankTwoTensor::Identity();
    _f_inv[_qp] = RankTwoTensor::Identity();
    const RankTwoTensor dL = _F[_qp] - _F_old[_qp];

    if (need_jacobian)
    {
      // d(dL)/dF = I^{(4)} when dL = F - F_old. d(dW)/dF = the skew projector.
      _d_deformation_gradient_increment_d_F[_qp] = RankFourTensor::IdentityFour();
      usingTensorIndices(i_, j_, k_, l_);
      const auto I2 = RankTwoTensor::Identity();
      _d_vorticity_increment_d_F[_qp] =
          0.5 * (RankFourTensor::IdentityFour() - I2.template times<j_, k_, i_, l_>(I2));

      // Small kinematics: R = I, U = I, dR/dF = 0. Defensive defaults; GN is not used here.
      _d_rotation_d_F[_qp].zero();
    }
    _rotation[_qp].setToIdentity();
    _stretch[_qp].setToIdentity();

    setQpIncrementalStrains(0.5 * (dL + dL.transpose()), 0.5 * (dL - dL.transpose()));
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpPolarDecomposition(bool need_jacobian)
{
  // Polar decomposition F = R * U of the alpha-weighted, F-bar-stabilized deformation
  // gradient at this qp. We decompose _F rather than _F_actual because the rest of the
  // kernel/rate chain treats _F as the spatial frame; using _F here matches the pre-3.1
  // GN rate exactly (it read _def_grad, which is _F via ComputeLagrangianStressCauchy).
  const RankTwoTensor & F = _F[_qp];
  FactorizedRankTwoTensor C(F.transpose() * F);
  // Reuse the sqrt factorization (a tensor eigensolve) for both U and U^{-1}.
  const auto sqrt_C = MathUtils::sqrt(C);
  _stretch[_qp] = sqrt_C.get();
  const RankTwoTensor U_inv = sqrt_C.inverse().get();
  _rotation[_qp] = F * U_inv;

  if (!need_jacobian)
    return;

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
ComputeLagrangianStrainBase<G>::computeQpLargeKinematicIncrement(const RankTwoTensor & f_inv,
                                                                 RankTwoTensor & dd,
                                                                 RankTwoTensor & dw,
                                                                 RankFourTensor & d_dL_d_f_inv,
                                                                 RankFourTensor & d_dw_d_f_inv,
                                                                 bool need_jacobian)
{
  switch (_kinematic_approximation)
  {
    case KinematicApproximation::Linear:
      computeLinearIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv, need_jacobian);
      break;
    case KinematicApproximation::Quadratic:
      computeQuadraticIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv, need_jacobian);
      break;
    case KinematicApproximation::RashidApproximate:
      computeRashidApproximateIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv, need_jacobian);
      break;
    case KinematicApproximation::RashidEigen:
      computeRashidEigenIncrement(f_inv, dd, dw, d_dL_d_f_inv, d_dw_d_f_inv, need_jacobian);
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
  _deformation_gradient_increment[_qp] = dd + dw;

  // Increment the total strain
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  // Get rid of the eigenstrains
  // Note we currently do not alter the deformation gradient -- this will be
  // needed in the future for a "complete" system
  subtractQpEigenstrainIncrement(_strain_increment[_qp]);

  // Increment the mechanical strain
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

  // Additionally maintain the rotated mechanical-strain accumulator,
  // eps_n+1 = r_hat (eps_n + Deltad) r_hat^T, where r_hat = exp(Deltaw) via Rodrigues. This matches
  // the mechanical_strain output convention of `ComputeFiniteStrain` and is consumed only by aux
  // variables -- the stress chain still uses the un-rotated `_mechanical_strain` above. In small
  // kinematics dw is small and r_hat ~= I + dw + 1/2 dw^2, which contributes only second-order
  // corrections (equivalent to no rotation for small strain).
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

  // Published rotation increment for downstream `ComputeStressBase`-style materials. The
  // default (identity) preserves the historical Lagrangian pipeline where the objective
  // rate is the sole rotation source. With `publish_rotation_increment = true` we dispatch
  // to the `kinematic_approximation`-matched formula so wrapped plasticity with
  // `perform_finite_strain_rotations = true` rotates its `_stress` bit-for-bit like OLD's
  // `ComputeFiniteStrain` would have done (the rate then runs in `rotate_old_stress`
  // passthrough mode so we don't double-rotate). Specifically `rashid_approximate` uses
  // OLD's C1/C2/C3 polynomial form (not `exp(dw)`) so the wrapped material's accumulated
  // stress storage matches OLD's `_stress` byte-for-byte through return mapping; without
  // this, the small (~1e-5) per-step rotation drift between `exp(dw)` and OLD's R_incr
  // amplifies through plastic flow into ~1e-3 cumulative stress error.
  _rotation_increment[_qp] = (_publish_rotation_increment && _large_kinematics)
                                 ? computeQpRotationIncrement(_f_inv[_qp], dw)
                                 : RankTwoTensor::Identity();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeLinearIncrement(const RankTwoTensor & f_inv,
                                                       RankTwoTensor & dd,
                                                       RankTwoTensor & dw,
                                                       RankFourTensor & d_dL_d_f_inv,
                                                       RankFourTensor & d_dw_d_f_inv,
                                                       bool need_jacobian) const
{
  // Deltal = I - f^{-1}, so Deltad = sym(Deltal), Deltaw = skew(Deltal).
  const RankTwoTensor dL = RankTwoTensor::Identity() - f_inv;
  dd = 0.5 * (dL + dL.transpose());
  dw = 0.5 * (dL - dL.transpose());

  if (!need_jacobian)
    return;
  // d(Deltal)/d(f^{-1}) = -I^{(4)}.
  d_dL_d_f_inv = -RankFourTensor::IdentityFour();
  // d(Deltaw)/d(f^{-1}) = - skew projector on f^{-1} = -(1/2)(I^{ikjl} - I^{iljk}).
  usingTensorIndices(i_, j_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();
  d_dw_d_f_inv = -0.5 * (RankFourTensor::IdentityFour() - I2.template times<j_, m_, i_, n_>(I2));
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQuadraticIncrement(const RankTwoTensor & f_inv,
                                                          RankTwoTensor & dd,
                                                          RankTwoTensor & dw,
                                                          RankFourTensor & d_dL_d_f_inv,
                                                          RankFourTensor & d_dw_d_f_inv,
                                                          bool need_jacobian) const
{
  // Deltal = X + (1/2) X^2 with X = I - f^{-1} (one more Taylor term of -log f^{-1}).
  const RankTwoTensor X = RankTwoTensor::Identity() - f_inv;
  const RankTwoTensor dL = X + 0.5 * X * X;
  dd = 0.5 * (dL + dL.transpose());
  dw = 0.5 * (dL - dL.transpose());

  if (!need_jacobian)
    return;
  // dX/d(f^{-1}) = -I^{(4)}, and d(X^2)_{ij}/dX_{mn} = delta_{im} X_{nj} + X_{im} delta_{jn}.
  // So d(Deltal)/d(f^{-1}) = -I^{(4)} - (1/2) (delta_{im} X_{nj} + X_{im} delta_{jn}).
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
ComputeLagrangianStrainBase<G>::computeRashidApproximateIncrement(const RankTwoTensor & f_inv,
                                                                  RankTwoTensor & dd,
                                                                  RankTwoTensor & dw,
                                                                  RankFourTensor & d_dL_d_f_inv,
                                                                  RankFourTensor & d_dw_d_f_inv,
                                                                  bool need_jacobian) const
{
  // See plan_outline.pdf Sec.2.3 (eq 10-15, with the corrected vorticity).
  // X = I - f^{-1}.  Symmetric part: A = X X^T - X - X^T,  Deltad = -A/2 + A^2/4.
  // Skew part (from the rotation tensor): alpha_i = eps_ijk (f^{-1})_jk,
  //   cos theta = (tr(f^{-1}) - 1)/2, sin theta = sqrt(1 - cos^2theta),  Q = (1/4) alpha*alpha,
  //   Deltaw_ij = -(theta / (2sqrtQ)) eps_ijk alpha_k.
  usingTensorIndices(i_, j_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();
  const auto I4 = RankFourTensor::IdentityFour();

  // ---- symmetric part ----
  const RankTwoTensor X = I2 - f_inv;
  const RankTwoTensor Xt = X.transpose();
  const RankTwoTensor A = X * Xt - X - Xt;
  dd = -0.5 * A + 0.25 * A * A;

  // ---- skew part ----
  // alpha_i = eps_ijk (f^{-1})_jk  (axial vector of f^{-1}'s skew part, doubled).
  RealVectorValue alpha;
  for (unsigned int i = 0; i < 3; ++i)
  {
    Real ai = 0.0;
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        ai += PermutationTensor::eps(i, j, k) * f_inv(j, k);
    alpha(i) = ai;
  }
  // Derive theta from the axial vector's magnitude rather than from tr(f^{-1}): the trace
  // formula assumes f^{-1} is exactly a rotation, which is only true in the limit. Here
  // sin theta = sqrtQ (Q = |skew part|^2/4); a true rotation matches both, and an arbitrary f^{-1}
  // is projected onto its nearest "rotation-like" interpretation.
  const Real Q_raw = 0.25 * (alpha * alpha);
  // Clamp Q just below 1 so cos theta stays strictly positive (theta -> pi/2 is unphysical for one
  // step).
  const Real Q = std::min(Q_raw, 1.0 - 1.0e-12);

  // Small-angle fallback: when sin theta -> 0, theta/(2sqrtQ) -> 1/2 (L'Hopital) and Deltaw ->
  // -alpha/2, which matches sym/skew of the linear approximation. d(Deltaw)/d(f^{-1}) is the
  // antisymmetrizer (1/2)(delta_im delta_jn - delta_jm delta_in).
  const Real small_Q = 1.0e-12;
  if (Q < small_Q)
  {
    // Deltaw_ij = -(1/2) eps_ijk alpha_k
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        Real v = 0.0;
        for (unsigned int k = 0; k < 3; ++k)
          v += PermutationTensor::eps(i, j, k) * alpha(k);
        dw(i, j) = -0.5 * v;
      }
    // d(Deltaw)/d(f^{-1}) = -(1/2)(delta_{im} delta_{jn} - delta_{jm} delta_{in})
    if (need_jacobian)
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

    // Build d(Deltaw)/d(f^{-1}) analytically.
    //   Deltaw_ij = c(f^{-1}) * E_ij(f^{-1}),  E_ij = eps_ijk alpha_k,  c = -theta/(2 sin theta).
    //
    //   dalpha_k/d(f^{-1})_{mn} = eps_{kmn}  (Levi-Civita is constant).
    //   dQ/d(f^{-1})_{mn} = (1/2) alpha_k eps_{kmn}.
    //   With sin theta = sqrtQ : dsin theta/d(f^{-1})_{mn} = alpha_k eps_{kmn} / (4 sin theta).
    //   dtheta/d(f^{-1})_{mn} = dsin theta/d(f^{-1})_{mn} / cos theta.
    //   dc/dtheta = -(sin theta - theta cos theta)/(2 sin^2 theta); dc/d(f^{-1})_{mn} = dc/dtheta *
    //   dtheta/d(f^{-1})_{mn}
    //     = (theta cos theta - sin theta) alpha_k eps_{kmn} / (8 sin^3 theta cos theta).
    //   dE_ij/d(f^{-1})_{mn} = eps_{ijk} eps_{kmn}.
    //
    // Final:
    //   d(Deltaw)_{ij}/d(f^{-1})_{mn} = (dc/d(f^{-1})_{mn}) * E_ij + c * dE_ij/d(f^{-1})_{mn}.
    if (need_jacobian)
    {
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
                dE_dfinv += PermutationTensor::eps(i, j, k) * PermutationTensor::eps(k, m, n);
              d_dw_d_f_inv(i, j, m, n) = dc_dfinv * E_ij + coeff * dE_dfinv;
            }
        }
    }
  }

  if (!need_jacobian)
    return;

  // ---- symmetric-part derivative + assemble d(Deltal)/d(f^{-1}) (Jacobian only) ----
  // dX/d(f^{-1}) = -I^{(4)}.
  // d(X X^T)_{ij}/dX_{mn} = delta_{im} X_{jn} + X_{in} delta_{jm}  (from (X X^T)_{ij} = X_{ik}
  // X_{jk}). d(X^T)_{ij}/dX_{mn} = delta_{jm} delta_{in}.
  // -> d(A)/d(f^{-1}) = -[d(X X^T)/dX] + I^{(4)} + (transposed I^{(4)}).
  const RankFourTensor d_XXt_dX =
      I2.template times<i_, m_, j_, n_>(X) + X.template times<i_, n_, j_, m_>(I2);
  const RankFourTensor d_Xt_dX = I2.template times<j_, m_, i_, n_>(I2);
  const RankFourTensor dA_dfinv = -d_XXt_dX + I4 + d_Xt_dX;

  // d(A^2)_{ij}/dA_{mn} = delta_{im} A_{nj} + A_{im} delta_{jn}.
  const RankFourTensor d_AA_dA =
      I2.template times<i_, m_, n_, j_>(A) + A.template times<i_, m_, j_, n_>(I2);
  const RankFourTensor d_AA_dfinv = d_AA_dA * dA_dfinv;

  const RankFourTensor d_dd_dfinv = -0.5 * dA_dfinv + 0.25 * d_AA_dfinv;
  d_dL_d_f_inv = d_dd_dfinv + d_dw_d_f_inv;
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeRashidEigenIncrement(const RankTwoTensor & f_inv,
                                                            RankTwoTensor & dd,
                                                            RankTwoTensor & dw,
                                                            RankFourTensor & d_dL_d_f_inv,
                                                            RankFourTensor & d_dw_d_f_inv,
                                                            bool need_jacobian) const
{
  // See plan_outline.pdf Sec.2.4. Polar-decompose f^{-1} = r' u', with u' symmetric positive
  // definite and r' a proper rotation. The PDF's identity `log f^{-1} = -log d - log w`
  // only holds when log u and log r commute (i.e. for non-rotating deformation); in general
  // the right stretch of f^{-1} is u' = R * U^{-1} * R^T where R, U are the right polar of f.
  // So -log(u') = R * log U * R^T is the *spatial-frame* log strain, not the co-rotated
  // log U. We compute it that way first, then rotate by r' = R^T to land in the n-frame
  // so that _strain_increment = log U exactly. This is the strain measure the Rashid
  // objective stress update consumes (eq. 22, sigma_{n+1} = r_hat (sigma_n + Deltasigma) r_hat^T
  // expects Deltasigma in the n-frame).
  usingTensorIndices(a_, b_, m_, n_);
  const auto I2 = RankTwoTensor::Identity();

  // ---- polar decomposition of f^{-1} ----
  FactorizedRankTwoTensor cprime(f_inv.transpose() * f_inv);
  // Reuse the sqrt factorization (a tensor eigensolve) for both u and u^{-1}.
  const auto sqrt_cprime = MathUtils::sqrt(cprime);
  const RankTwoTensor u = sqrt_cprime.get();
  const RankTwoTensor u_inv = sqrt_cprime.inverse().get();
  const RankTwoTensor r = f_inv * u_inv;

  // ---- intermediate Deltad_spatial = -log u' = R * log U * R^T (n+1 frame) ----
  // u' = sqrt(c'), so log(u') = (1/2) log(c'). Reuse c''s already-computed factorization
  // (`sqrt_cprime` shares c''s eigenvectors) instead of re-decomposing u -- one fewer 3x3
  // eigensolve per qp, on both residual and Jacobian sweeps.
  const RankTwoTensor dd_spatial = -0.5 * MathUtils::log(cprime).get();

  // ---- Deltaw = -log r via Rodrigues. ----
  // log r = phi(theta) (r - r^T) with phi(theta) = theta/(2 sin theta),  cos theta = (tr r - 1)/2.
  const Real cos_theta = MathUtils::clamp(0.5 * (r.trace() - 1.0), -1.0, 1.0);
  const Real sin2 = std::max(1.0 - cos_theta * cos_theta, 0.0);
  const Real sin_theta = std::sqrt(sin2);
  const Real theta = std::acos(cos_theta);
  const RankTwoTensor A = r - r.transpose();

  // d(log r)_{ij}/d(r)_{mn} = (dphi/dr_{mn}) A_{ij} + phi (delta_{im} delta_{jn} - delta_{jm}
  // delta_{in}).
  //   dphi/dr_{mn} = (dphi/dtheta)(dtheta/d cos theta)(d cos theta/dr_{mn})
  //              = psi delta_{mn}, where psi = (theta cos theta - sin theta)/(4 sin^3 theta).
  // Small-angle: phi -> 1/2, psi -> -1/12, so d(log r)/dr -> (1/2)(I^(4) - swap_ij).
  // `d_logr_d_r` feeds only the Jacobian, so it is built only when `need_jacobian`.
  const Real small_sin = 1.0e-7;
  RankFourTensor d_logr_d_r;
  RankTwoTensor log_r;
  usingTensorIndices(i_, j_, k_, l_);
  if (std::abs(sin_theta) < small_sin)
  {
    log_r = 0.5 * A;
    if (need_jacobian)
    {
      const RankFourTensor swap_ij = I2.template times<j_, m_, i_, n_>(I2);
      d_logr_d_r = 0.5 * (RankFourTensor::IdentityFour() - swap_ij);
    }
  }
  else
  {
    const Real phi = theta / (2.0 * sin_theta);
    log_r = phi * A;
    if (need_jacobian)
    {
      const Real psi = (theta * cos_theta - sin_theta) / (4.0 * sin_theta * sin2);
      const RankFourTensor swap_ij = I2.template times<j_, m_, i_, n_>(I2);
      // (dphi/dr)_{mn} = psi delta_{mn} -> outer with A_{ij} gives A.times<i_, j_, m_, n_>(I2) *
      // psi.
      const RankFourTensor dphi_outer_A = A.template times<i_, j_, m_, n_>(I2);
      d_logr_d_r = psi * dphi_outer_A + phi * (RankFourTensor::IdentityFour() - swap_ij);
    }
  }
  dw = -log_r;

  // ---- Rotate Deltad from spatial back to co-rotated (n) frame: log U = r' * (R log U R^T) *
  // r'^T, using r' = R^T from the polar of f^{-1}. ----
  dd = r * dd_spatial * r.transpose();

  // Everything below is the Jacobian chain (RankFour matrix-log derivatives, the
  // polar-decomposition dr/d(f^{-1}) closed form, and the three-piece sandwich chain rule).
  // Skip it wholesale on residual-only sweeps.
  if (!need_jacobian)
    return;

  // d(log u)/d(c') = (1/2) dlog(c'), and d(c')_{ab}/d(f^{-1})_{mn} = delta_{an} f_inv_{mb}
  //                                                                + delta_{bn} f_inv_{ma}.
  const RankFourTensor dlog_cprime = MathUtils::dlog(cprime);
  const RankFourTensor d_cprime_d_finv =
      I2.template times<a_, n_, m_, b_>(f_inv) + I2.template times<b_, n_, m_, a_>(f_inv);
  const RankFourTensor d_dd_spatial_d_finv = -0.5 * (dlog_cprime * d_cprime_d_finv);

  // ---- d(r)/d(f^{-1}) via the polar-decomposition closed form (same trick as
  //      ComputeLagrangianObjectiveStress::polarDecomposition). ----
  const RankTwoTensor Y = u.trace() * I2 - u;
  const RankTwoTensor Z = r * Y;
  const RankTwoTensor O = Z * r.transpose();
  const RankFourTensor d_r_d_finv =
      (O.template times<i_, k_, l_, j_>(Y) - Z.template times<i_, l_, k_, j_>(Z)) / Y.det();

  d_dw_d_f_inv = -(d_logr_d_r * d_r_d_finv);

  // The derivative of the sandwich r'*A*r'^T w.r.t. f^{-1} expands to three rank-4 pieces
  // (chain rule on r' AND on A). The 6-argument `times<>(RankFourTensor)` overload uses
  // x[0..4] (one dummy at index 4), so we reuse the same dummy label `p2_` in each
  // sub-contraction.
  {
    usingTensorIndices(i2_, j2_, m2_, n2_, p2_);
    const RankTwoTensor M = dd_spatial * r.transpose(); // dd * r^T   (p, j) shape
    const RankTwoTensor N = r * dd_spatial;             // r * dd     (i, q) shape
    // T1_{ijmn} = (dr/d_finv)_{ip,mn} * M_{pj}
    const RankFourTensor T1 = M.template times<p2_, j2_, i2_, p2_, m2_, n2_>(d_r_d_finv);
    // T2_{ijmn} = r_{ip} * (d_dd_spatial_d_finv)_{pq,mn} * r_{jq}: contract twice over the dummy.
    const RankFourTensor mid = r.template times<i2_, p2_, p2_, j2_, m2_, n2_>(d_dd_spatial_d_finv);
    const RankFourTensor T2 = r.template times<j2_, p2_, i2_, p2_, m2_, n2_>(mid);
    // T3_{ijmn} = N_{iq} * (dr/d_finv)_{jq,mn}
    const RankFourTensor T3 = N.template times<i2_, p2_, j2_, p2_, m2_, n2_>(d_r_d_finv);
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

  // The F-bar local/non-local derivative material properties feed only the Jacobian path
  // (the stress materials chain them into `_pk1_jacobian`/`_cauchy_jacobian`, and the
  // TL kernel's non-local F-bar term contracts `_d_F_stab_d_F_avg`). The residual sweep
  // never reads them, so skip the R4 algebra when assembling a residual alone.
  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();

  // If stabilization is on do the volumetric correction
  if (_stabilize_strain)
  {
    // `_F_avg` consistently stores the element average of `F_ust` regardless of mode;
    // it's consumed by the UL kernel for the spatial-frame push-forward, which is a
    // purely-geometric quantity. In `F_bar_mode = incremental` the F-bar chain itself
    // operates on the *incremental* averaged tensor `f_avg = avg(F_ust * F_ust_old^{-1})`,
    // which we compute locally -- the kernel matches by weighting grad_phi by F_ust_old^{-1}
    // in its element average. Incremental mode only makes sense for large kinematics
    // (OLD's Fhat F-bar lives in the finite-strain code path).
    const bool incremental = (_F_bar_mode == FBarMode::Incremental);
    if (incremental && !_large_kinematics)
      mooseError("`F_bar_mode = incremental` requires `large_kinematics = true`. The "
                 "incremental F-bar formulation is the multiplicative correction to the "
                 "incremental F, which is only defined for large kinematics. Use "
                 "`F_bar_mode = total` (the default) with small kinematics.");
    const auto F_avg = StabilizationUtils::elementAverage(
        [this](unsigned int qp) { return _F_ust[qp]; }, _JxW, _coord);
    const auto f_avg =
        incremental
            ? StabilizationUtils::elementAverage([this](unsigned int qp)
                                                 { return _F_ust[qp] * _F_ust_old[qp].inverse(); },
                                                 _JxW,
                                                 _coord)
            : RankTwoTensor();
    // Always publish avg(F_ust) -- UL kernel uses this for the push-forward.
    _F_avg.set().setAllValues(F_avg);
    // What the F-bar gamma and `_d_F_stab_d_F_avg` are built against (also what the
    // kernel-side `_avg_grad_trial` will represent the delta of):
    const auto & avg_for_chain = incremental ? f_avg : F_avg;
    // Make the appropriate modification, depending on small or large deformations
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (_large_kinematics)
      {
        // Multiplicative F-bar: F_stab = gamma * F_ust where
        //   gamma_total = cbrt(det(F_avg) / det(F_ust))                       (total mode)
        //   gamma_inc   = cbrt(det(f_avg) / det(f_ust)),  f_ust = F_ust*F_ust_old^{-1}
        //                                                                     (incremental mode)
        // For incremental:  det(f_ust) = det(F_ust)/det(F_ust_old).
        //   d log det(f_ust)/d F_ust = F_ust^{-T}   (same as the total-mode chain)
        // so `_d_F_stab_d_F_ust` shares the total-mode shape. The non-local chain
        // contracts `_d_F_stab_d_F_avg` with delta(averaged-quantity); kernel computes
        // deltaf_avg in incremental mode by weighting grad_phi by F_ust_old^{-1} in its
        // element average.
        const Real det_ust_local =
            incremental ? _F_ust[_qp].det() / _F_ust_old[_qp].det() : _F[_qp].det();
        const Real gamma = std::pow(avg_for_chain.det() / det_ust_local, 1.0 / 3.0);
        if (need_jacobian)
        {
          const auto Fust_invT = _F_ust[_qp].inverse().transpose();
          const auto avg_invT = avg_for_chain.inverse().transpose();
          _d_F_stab_d_F_ust[_qp] =
              gamma * RankFourTensor::IdentityFour() -
              (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(Fust_invT);
          _d_F_stab_d_F_avg[_qp] =
              (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(avg_invT);
        }
        _F[_qp] *= gamma;
      }
      else
      {
        if (need_jacobian)
        {
          // Additive (trace) F-bar: F_stab = F_ust + (tr(F_avg - F_ust)/3) * I.
          // dF_stab/dF_ust = I^(4) - (1/3) * I2 (x) I2  (each diagonal component pulled out).
          // dF_stab/dF_avg = (1/3) * I2 (x) I2.
          const auto outer = I2.template times<i_, j_, k_, l_>(I2);
          _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour() - (1.0 / 3.0) * outer;
          _d_F_stab_d_F_avg[_qp] = (1.0 / 3.0) * outer;
        }
        _F[_qp] += (F_avg.trace() - _F[_qp].trace()) * I2 / 3.0;
      }
    }
  }
  else if (need_jacobian)
  {
    // F-bar off: dF_stab/dF_ust = I^(4), dF_stab/dF_avg = 0.
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour();
      _d_F_stab_d_F_avg[_qp].zero();
    }
  }
}

template <class G>
RankTwoTensor
ComputeLagrangianStrainBase<G>::computeQpRotationIncrement(const RankTwoTensor & f_inv,
                                                           const RankTwoTensor & dw) const
{
  // For `rashid_approximate` port OLD `ComputeFiniteStrain`'s C1/C2/C3 polynomial form
  // exactly (Rashid 1993). Pairing this rotation with the wrapped material's FSR (via
  // `_rotation_increment`) lets the constitutive's `_stress` evolution match OLD's bit-
  // for-bit at the same converged displacement state.
  if (_kinematic_approximation == KinematicApproximation::RashidApproximate)
  {
    const Real a[3] = {
        f_inv(1, 2) - f_inv(2, 1), f_inv(2, 0) - f_inv(0, 2), f_inv(0, 1) - f_inv(1, 0)};
    const Real q = (a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) / 4.0;
    const Real trFhatinv_1 = f_inv.trace() - 1.0;
    const Real p = trFhatinv_1 * trFhatinv_1 / 4.0;
    const Real C1_squared = p +
                            3.0 * Utility::pow<2>(p) * (1.0 - (p + q)) / Utility::pow<2>(p + q) -
                            2.0 * Utility::pow<3>(p) * (1.0 - (p + q)) / Utility::pow<3>(p + q);
    if (C1_squared <= 0.0)
      mooseException(
          "Cannot take square root of a number less than or equal to zero in the calculation of "
          "C1 for the Rashid approximation for the rotation tensor.");
    const Real C1 = std::sqrt(C1_squared);
    Real C2;
    if (q > 0.01)
      C2 = (1.0 - C1) / (4.0 * q);
    else
      C2 = 0.125 + q * 0.03125 * (Utility::pow<2>(p) - 12.0 * (p - 1.0)) / Utility::pow<2>(p) +
           Utility::pow<2>(q) * (p - 2.0) * (Utility::pow<2>(p) - 10.0 * p + 32.0) /
               Utility::pow<3>(p) +
           Utility::pow<3>(q) *
               (1104.0 - 992.0 * p + 376.0 * Utility::pow<2>(p) - 72.0 * Utility::pow<3>(p) +
                5.0 * Utility::pow<4>(p)) /
               (512.0 * Utility::pow<4>(p));
    const Real C3_test =
        (p * q * (3.0 - q) + Utility::pow<3>(p) + Utility::pow<2>(q)) / Utility::pow<3>(p + q);
    if (C3_test <= 0.0)
      mooseException(
          "Cannot take square root of a number less than or equal to zero in the calculation of "
          "C3_test for the Rashid approximation for the rotation tensor.");
    const Real C3 = 0.5 * std::sqrt(C3_test);
    RankTwoTensor R_incr;
    R_incr.addIa(C1);
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        R_incr(i, j) += C2 * a[i] * a[j];
    R_incr(0, 1) += C3 * a[2];
    R_incr(0, 2) -= C3 * a[1];
    R_incr(1, 0) -= C3 * a[2];
    R_incr(1, 2) += C3 * a[0];
    R_incr(2, 0) += C3 * a[1];
    R_incr(2, 1) -= C3 * a[0];
    return R_incr.transpose();
  }

  // For `rashid_eigen`, `linear`, `quadratic`: r_hat = exp(dw) via Rodrigues. For RashidEigen,
  // dw is the matrix log of the polar-decomposition R, so exp(dw) recovers that R bit-for-bit
  // -- equivalent to OLD `ComputeFiniteStrain`'s EigenSolution rotation.
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
  return RankTwoTensor::Identity() + f * dw + g * dw * dw;
}

template class ComputeLagrangianStrainBase<GradientOperatorCartesian>;
template class ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>;
template class ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>;
