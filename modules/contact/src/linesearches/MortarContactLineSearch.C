//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactLineSearch.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "WeightedGapUserObject.h"
#include "LMWeightedGapUserObject.h"
#include "LMWeightedVelocitiesUserObject.h"
#include "MooseVariableFE.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_solver_exception.h"

registerMooseObject("ContactApp", MortarContactLineSearch);

namespace
{
/// Whether a ConstraintStateDiff records no changes at all.
bool
diffIsEmpty(const Moose::Mortar::Contact::ConstraintStateDiff & diff)
{
  return diff.newly_open.empty() && diff.newly_contact.empty() && diff.newly_stick.empty() &&
         diff.newly_slip.empty();
}
}

InputParameters
MortarContactLineSearch::validParams()
{
  InputParameters params = ContactLineSearchBase::validParams();
  params.addRequiredParam<UserObjectName>(
      "weighted_gap_uo",
      "The weighted-gap user object providing the per-dof weighted gap and normalization; must "
      "match the corresponding mortar [Constraints] block's 'weighted_gap_uo' parameter.");
  params.addParam<UserObjectName>(
      "weighted_velocities_uo",
      "The weighted-velocities user object providing per-dof tangential velocities. When given, "
      "dofs are classified using the frictional stick/slip switch instead of the frictionless "
      "open/closed switch.");
  params.addRequiredParam<VariableName>(
      "lm_variable", "The Lagrange multiplier variable enforcing the normal contact constraint.");
  params.addParam<VariableName>(
      "friction_lm_variable",
      "The frictional Lagrange multiplier variable. Required when 'weighted_velocities_uo' is "
      "given.");
  params.addParam<VariableName>(
      "friction_lm_dir_variable",
      "The second frictional Lagrange multiplier variable (3D problems only). Required in 3D "
      "when 'weighted_velocities_uo' is given.");
  params.addParam<Real>("c",
                        1e0,
                        "Numerical parameter multiplying the weighted gap; must match the "
                        "corresponding [Constraints] block's 'c' parameter.");
  params.addParam<bool>("normalize_c",
                        false,
                        "Whether to normalize 'c' by the accumulated mortar weight; must match "
                        "the corresponding [Constraints] block.");
  params.addParam<bool>(
      "use_derived_c_normal",
      false,
      "Whether to read the normal stiffness scale per-dof from the weighted-gap user object's "
      "dofToDerivedC() instead of 'c'; must match the corresponding [Constraints] block.");
  params.addParam<Real>("c_t",
                        1e0,
                        "Numerical parameter for the tangential constraint; must match the "
                        "corresponding [Constraints] block's 'c_t' parameter. Only used when "
                        "'weighted_velocities_uo' is given. The corresponding [Constraints] "
                        "block must leave 'dynamic_c_t' at its default (false); that option is "
                        "not supported by this class.");
  params.addParam<Real>("mu",
                        "The constant Coulomb friction coefficient. Required when "
                        "'weighted_velocities_uo' is given. Must match the corresponding "
                        "[Constraints] block's constant 'mu'; a Function-valued friction "
                        "coefficient is not supported by this class.");
  params.addParam<Real>(
      "epsilon",
      "Below this raw normal Lagrange multiplier value a dof is classified OPEN regardless of "
      "its frictional switch value; must match the corresponding [Constraints] block's "
      "'epsilon' parameter. Required when 'weighted_velocities_uo' is given.");
  params.addParam<Real>(
      "direct_accept_tol",
      1e-2,
      "Fractional residual-norm reduction (relative to the checkpointed norm) the backing line "
      "search must achieve for its result to be accepted directly once the contact set has "
      "changed.");
  params.addParam<Real>(
      "event_group_tol",
      1e-2,
      "Step-length tolerance used to group near-simultaneous predicted normal-switch events.");
  return params;
}

MortarContactLineSearch::MortarContactLineSearch(const InputParameters & parameters)
  : ContactLineSearchBase(parameters),
    _c(getParam<Real>("c")),
    _normalize_c(getParam<bool>("normalize_c")),
    _use_derived_c_normal(getParam<bool>("use_derived_c_normal")),
    _c_t(getParam<Real>("c_t")),
    _direct_accept_tol(getParam<Real>("direct_accept_tol")),
    _event_group_tol(getParam<Real>("event_group_tol"))
{
}

void
MortarContactLineSearch::initialSetup()
{
  const UserObject & gap_uo =
      _fe_problem.getUserObjectBase(getParam<UserObjectName>("weighted_gap_uo"));
  _weighted_gap_uo = dynamic_cast<const WeightedGapUserObject *>(&gap_uo);
  if (!_weighted_gap_uo)
    paramError("weighted_gap_uo", "The supplied user object must derive from WeightedGapUserObject.");

  if (isParamValid("weighted_velocities_uo"))
  {
    const UserObject & vel_uo =
        _fe_problem.getUserObjectBase(getParam<UserObjectName>("weighted_velocities_uo"));
    _weighted_velocities_uo = dynamic_cast<const LMWeightedVelocitiesUserObject *>(&vel_uo);
    if (!_weighted_velocities_uo)
      paramError("weighted_velocities_uo",
                 "The supplied user object must derive from LMWeightedVelocitiesUserObject.");

    if (!isParamValid("friction_lm_variable"))
      paramError("friction_lm_variable", "This parameter is required when 'weighted_velocities_uo' is given.");
    if (!isParamValid("mu"))
      paramError("mu", "This parameter is required when 'weighted_velocities_uo' is given.");
    if (!isParamValid("epsilon"))
      paramError("epsilon", "This parameter is required when 'weighted_velocities_uo' is given.");
    _mu = getParam<Real>("mu");
    _epsilon = getParam<Real>("epsilon");

    _friction_lm_vars.push_back(
        &_fe_problem.getStandardVariable(0, getParam<VariableName>("friction_lm_variable")));
    if (isParamValid("friction_lm_dir_variable"))
      _friction_lm_vars.push_back(&_fe_problem.getStandardVariable(
          0, getParam<VariableName>("friction_lm_dir_variable")));
  }

  if (_use_derived_c_normal)
  {
    // LMWeightedVelocitiesUserObject derives directly from LMWeightedGapUserObject, so when
    // friction is enabled the same user object already provides dofToDerivedC().
    const LMWeightedGapUserObject * lm_gap_uo =
        _weighted_velocities_uo
            ? static_cast<const LMWeightedGapUserObject *>(_weighted_velocities_uo)
            : dynamic_cast<const LMWeightedGapUserObject *>(_weighted_gap_uo);
    if (!lm_gap_uo)
      paramError("use_derived_c_normal",
                 "The weighted-gap user object must derive from LMWeightedGapUserObject to use "
                 "this option.");
    _lm_weighted_gap_uo = lm_gap_uo;
  }

  _lm_var = &_fe_problem.getStandardVariable(0, getParam<VariableName>("lm_variable"));
}

MortarContactLineSearch::Classification
MortarContactLineSearch::classify(const Vec & solution) const
{
  using namespace Moose::Mortar::Contact;

  Classification result;

  PetscInt low, high;
  LibmeshPetscCall(VecGetOwnershipRange(solution, &low, &high));
  const PetscScalar * solution_array;
  LibmeshPetscCall(VecGetArrayRead(solution, &solution_array));

  auto solutionValue = [&](dof_id_type dof_index) -> ADReal
  {
    mooseAssert(static_cast<PetscInt>(dof_index) >= low && static_cast<PetscInt>(dof_index) < high,
                "classify() only reads dofs owned by this rank");
    ADReal value = solution_array[dof_index - low];
    Moose::derivInsert(value.derivatives(), dof_index, 1.);
    return value;
  };

  const Real dt = _fe_problem.dt();

  for (const auto & [dof_object, gap_pr] : _weighted_gap_uo->dofToWeightedGap())
  {
    if (dof_object->processor_id() != processor_id())
      continue;

    const auto & [weighted_gap, normalization] = gap_pr;
    const auto dof_id = dof_object->id();

    const auto lm_dof_index = dof_object->dof_number(_nl.number(), _lm_var->number(), 0);
    const ADReal lm_value = solutionValue(lm_dof_index);

    // Mirrors ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof's resolution of the
    // normal reference stiffness. This is the switch value Component B predicts events from, and
    // is computed unconditionally regardless of whether the dof ends up classified below via the
    // frictionless or frictional path.
    const Real normal_scale =
        _use_derived_c_normal ? libmesh_map_find(_lm_weighted_gap_uo->dofToDerivedC(), dof_object)[0]
                              : _c;
    const Real c =
        (_use_derived_c_normal || _normalize_c) ? normal_scale / normalization : normal_scale;
    result.normal_switch_values.emplace(dof_id, lm_value - c * weighted_gap);

    ConstraintState state;
    if (_weighted_velocities_uo)
    {
      // Mirrors ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof(3d)'s own,
      // separately-resolved normal and tangential reference stiffnesses: unlike 'c' above, this
      // only divides by normalization when 'use_derived_c_normal' is false or 'normalize_c' is
      // true, exactly matching the source formula it is transcribed from.
      Real c_use;
      if (_use_derived_c_normal)
      {
        const Real c_nn = libmesh_map_find(_lm_weighted_gap_uo->dofToDerivedC(), dof_object)[0];
        c_use = _normalize_c ? c_nn / normalization : c_nn;
      }
      else
        c_use = _normalize_c ? _c / normalization : _c;
      const Real c_t_use = _normalize_c ? _c_t / normalization : _c_t;

      const auto & weighted_velocities =
          libmesh_map_find(_weighted_velocities_uo->dofToWeightedVelocities(), dof_object);

      const ADReal mu_ad(_mu);
      const ADReal augmented_normal_pressure = lm_value + c_use * weighted_gap;
      const ADReal radius = coulombFrictionRadius(mu_ad, augmented_normal_pressure);
      const ADReal epsilon_ad(_epsilon);

      const auto friction_dof_index =
          dof_object->dof_number(_nl.number(), _friction_lm_vars[0]->number(), 0);
      const ADReal friction_lm_value = solutionValue(friction_dof_index);

      if (_friction_lm_vars.size() == 1)
      {
        const ADReal augmented_tangential_pressure =
            friction_lm_value + c_t_use * weighted_velocities[0] * dt;
        state =
            classifyFrictionalState(augmented_tangential_pressure, radius, lm_value, epsilon_ad);
      }
      else
      {
        const auto friction_dir_dof_index =
            dof_object->dof_number(_nl.number(), _friction_lm_vars[1]->number(), 0);
        const ADReal friction_lm_dir_value = solutionValue(friction_dir_dof_index);
        const std::array<ADReal, 2> augmented_tangential_pressure{
            {friction_lm_value + c_t_use * weighted_velocities[0] * dt,
             friction_lm_dir_value + c_t_use * weighted_velocities[1] * dt}};
        state =
            classifyFrictionalState3d(augmented_tangential_pressure, radius, lm_value, epsilon_ad);
      }
    }
    else
      state = classifyNormalState(lm_value, weighted_gap, c);

    result.states.emplace(dof_id, state);
  }

  LibmeshPetscCall(VecRestoreArrayRead(solution, &solution_array));
  return result;
}

void
MortarContactLineSearch::lineSearch()
{
  using namespace Moose::Mortar::Contact;

  setupBackingLineSearch();

  Vec X, F, Y, W, G;
  SNESLineSearch line_search;
  PetscReal fnorm0, xnorm0, ynorm0;
  SNES snes = _solver->snes();

  LibmeshPetscCall(SNESGetLineSearch(snes, &line_search));
  LibmeshPetscCall(SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G));
  LibmeshPetscCall(SNESLineSearchGetNorms(line_search, &xnorm0, &fnorm0, &ynorm0));
  LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED));

  ++_nl_its;

  // Checkpoint: classify the incoming iterate before the backing search perturbs anything.
  const auto checkpoint = classify(X);

  if (_affect_ltol)
  {
    KSP ksp;
    PetscReal ksp_rtol, ksp_abstol, ksp_dtol;
    PetscInt ksp_maxits;
    LibmeshPetscCall(SNESGetKSP(snes, &ksp));
    LibmeshPetscCall(KSPGetTolerances(ksp, &ksp_rtol, &ksp_abstol, &ksp_dtol, &ksp_maxits));

    if (!_user_ksp_rtol_set)
    {
      _user_ksp_rtol = ksp_rtol;
      _user_ksp_rtol_set = true;
    }

    // _old_state is empty on the very first call, before any canonical state has been recorded;
    // treat that as "changing" since there is nothing yet to compare against.
    const bool contact_set_changing =
        _old_state.empty() || !diffIsEmpty(symmetricDifference(_old_state, checkpoint.states));
    if (contact_set_changing)
      LibmeshPetscCall(KSPSetTolerances(ksp, _contact_ltol, ksp_abstol, ksp_dtol, ksp_maxits));
    else
      LibmeshPetscCall(KSPSetTolerances(ksp, _user_ksp_rtol, ksp_abstol, ksp_dtol, ksp_maxits));
  }

  // Backing invocation: run the standard PETSc line search on working-copy vectors so the outer
  // X/F/Y stay untouched until a branch below deliberately commits.
  Vec Xb, Fb, Yb;
  LibmeshPetscCall(VecDuplicate(X, &Xb));
  LibmeshPetscCall(VecDuplicate(F, &Fb));
  LibmeshPetscCall(VecDuplicate(Y, &Yb));
  LibmeshPetscCall(VecCopy(X, Xb));
  LibmeshPetscCall(VecCopy(F, Fb));
  LibmeshPetscCall(VecCopy(Y, Yb));

  // SNESLineSearchApply's fnorm argument is in/out: a non-null pointer tells it the norm of the
  // incoming F is already known, skipping its own VecNorm call. Fb was just copied from F, whose
  // norm is exactly fnorm0, so seed it rather than leaving it uninitialized garbage.
  PetscReal fnorm_b = fnorm0;
  LibmeshPetscCall(SNESLineSearchApply(_backing_petsc_line_search, Xb, Fb, &fnorm_b, Yb));
  SNESLineSearchReason backing_reason;
  LibmeshPetscCall(SNESLineSearchGetReason(_backing_petsc_line_search, &backing_reason));
  const bool backing_succeeded = (backing_reason == SNES_LINESEARCH_SUCCEEDED);

  // Trust the backing search's own Xb/Fb/fnorm_b directly; SNESLineSearchApply already refreshed
  // them via its own internal residual evaluations, so no further SNESComputeFunction call is
  // needed here.
  const auto state_backing = classify(Xb);
  const auto diff = symmetricDifference(checkpoint.states, state_backing.states);
  bool identity_changed = !diffIsEmpty(diff);
  comm().max(identity_changed);

  if (!identity_changed)
  {
    // Unchanged identity: nothing contact-related changed, so pass the backing search's result
    // through exactly as it produced it -- including its own reason, even a failure, for full
    // transparency when the contact set is stable.
    LibmeshPetscCall(VecCopy(Xb, X));
    LibmeshPetscCall(VecCopy(Fb, F));
    LibmeshPetscCall(VecCopy(Yb, Y));
    LibmeshPetscCall(SNESLineSearchSetReason(line_search, backing_reason));
    _old_state = state_backing.states;
  }
  else if (backing_succeeded && fnorm_b < fnorm0 * (1 - _direct_accept_tol))
  {
    // Direct acceptance: the contact set changed, but the backing search still achieved enough
    // residual reduction on its own to trust its result outright.
    LibmeshPetscCall(VecCopy(Xb, X));
    LibmeshPetscCall(VecCopy(Fb, F));
    LibmeshPetscCall(VecCopy(Yb, Y));
    LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED));
    _old_state = state_backing.states;
  }
  else
  {
    // Event-limited fallback prediction (Component B): estimate, for each dof, the fraction of
    // the full Newton step at which its normal switch value q_n = lm_value - c*weighted_gap is
    // predicted to cross zero. This is a secant between the two points already classified above
    // -- q0 at the checkpoint and q_b at the backing search's proposed iterate -- rather than an
    // analytic tangent at the checkpoint, because weighted_gap's cached AD derivatives are not
    // usable here: residual-only evaluations run with AD derivative tracking disabled (a global
    // performance optimization), so the weighted gap this reads was never given displacement-dof
    // sensitivities to begin with. Rescaling by the backing search's actual step fraction
    // (SNESLineSearchGetLambda) puts the secant slope onto the same fraction-of-full-Newton-step
    // convention eventStepLength()/firstEventGroup() expect.
    PetscReal lambda_b;
    LibmeshPetscCall(SNESLineSearchGetLambda(_backing_petsc_line_search, &lambda_b));

    std::unordered_map<dof_id_type, Real> predicted_alphas;
    for (const auto & [dof_id, q0] : checkpoint.normal_switch_values)
    {
      const Real q0_raw = MetaPhysicL::raw_value(q0);
      const Real qb_raw =
          MetaPhysicL::raw_value(libmesh_map_find(state_backing.normal_switch_values, dof_id));
      const Real qdot_secant = (qb_raw - q0_raw) / lambda_b;
      if (const auto alpha = eventStepLength(q0_raw, qdot_secant))
        predicted_alphas.emplace(dof_id, *alpha);
    }
    const auto event_group = firstEventGroup(predicted_alphas, _event_group_tol);

    if (event_group)
    {
      // Event-limited fallback: take a single step to the first predicted normal-switch event
      // instead of trusting the backing search's rejected iterate. X has not been touched above,
      // so this steps from the checkpoint.
      const Real step = std::min(Real(1), event_group->alpha_min);
      LibmeshPetscCall(VecWAXPY(W, -step, Y, X));
      LibmeshPetscCall(SNESComputeFunction(snes, W, F));
#if PETSC_VERSION_LESS_THAN(3, 25, 0)
      PetscBool domainerror;
      LibmeshPetscCall(SNESGetFunctionDomainError(snes, &domainerror));
      if (domainerror)
        LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN));
#else
      SNESConvergedReason reason;
      LibmeshPetscCall(SNESGetConvergedReason(snes, &reason));
      if (reason == SNES_DIVERGED_FUNCTION_DOMAIN)
        LibmeshPetscCall(
            SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_FUNCTION_DOMAIN));
#endif
      LibmeshPetscCall(VecCopy(W, X));
      LibmeshPetscCall(VecScale(Y, step));
      _old_state = classify(X).states;
    }
    else
    {
      // Propagate failure: the contact set changed, direct acceptance failed, and no event is
      // predicted to bound a fallback step. Report the backing search's failure (or a generic
      // insufficient-reduction reason if it nominally succeeded) and leave X/F/Y untouched.
      LibmeshPetscCall(SNESLineSearchSetReason(
          line_search, backing_succeeded ? SNES_LINESEARCH_FAILED_REDUCT : backing_reason));
      _old_state = checkpoint.states;
    }
  }

  LibmeshPetscCall(VecDestroy(&Xb));
  LibmeshPetscCall(VecDestroy(&Fb));
  LibmeshPetscCall(VecDestroy(&Yb));

  LibmeshPetscCall(SNESLineSearchComputeNorms(line_search));
}
