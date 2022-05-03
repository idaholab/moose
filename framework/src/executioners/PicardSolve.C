//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PicardSolve.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

InputParameters
PicardSolve::validParams()
{
  InputParameters params = FixedPointSolve::validParams();

  params.addDeprecatedParam<unsigned int>(
      "picard_max_its",
      1,
      "Specifies the maximum number of Picard iterations. "
      "Mainly used when  wanting to do Picard iterations with MultiApps "
      "that are set to execute_on timestep_end or timestep_begin. "
      "Setting this parameter to 1 turns off the Picard iterations.",
      "Deprecated, use fixed_point_max_its");
  params.addDeprecatedParam<bool>(
      "accept_on_max_picard_iteration",
      false,
      "True to treat reaching the maximum number of Picard iterations as converged.",
      "Deprecated, use accept_on_max_fixed_point_iteration");
  params.addDeprecatedParam<bool>(
      "disable_picard_residual_norm_check",
      false,
      "Disable the Picard residual norm evaluation thus the three parameters "
      "picard_rel_tol, picard_abs_tol and picard_force_norms.",
      "Deprecated, use disable_fixed_point_residual_norm_check");
  params.addDeprecatedParam<Real>("picard_rel_tol",
                                  1e-8,
                                  "The relative nonlinear residual drop to shoot for "
                                  "during Picard iterations. This check is "
                                  "performed based on the Master app's nonlinear "
                                  "residual.",
                                  "Deprecated, use fixed_point_rel_tol");
  params.addDeprecatedParam<Real>("picard_abs_tol",
                                  1e-50,
                                  "The absolute nonlinear residual to shoot for "
                                  "during Picard iterations. This check is "
                                  "performed based on the Master app's nonlinear "
                                  "residual.",
                                  "Deprecated, use fixed_point_abs_tol");
  params.addDeprecatedParam<PostprocessorName>("picard_custom_pp",
                                               "Postprocessor for custom picard convergence check.",
                                               "Deprecated, use custom_pp");

  params.addDeprecatedParam<bool>(
      "picard_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existence of active MultiApps with those execute_on flags, default: false.",
      "Deprecated, use fixed_point_force_norms");

  return params;
}

PicardSolve::PicardSolve(Executioner & ex) : FixedPointSolve(ex)
{
  // Handle deprecated parameters
  if (!parameters().isParamSetByAddParam("picard_max_its"))
  {
    _max_fixed_point_its = getParam<unsigned int>("picard_max_its");
    _has_fixed_point_its = _max_fixed_point_its > 1;
  }

  if (!parameters().isParamSetByAddParam("accept_on_max_picard_iteration"))
    _accept_max_it = getParam<bool>("accept_on_max_picard_iteration");

  if (!parameters().isParamSetByAddParam("disable_picard_residual_norm_check"))
    _has_fixed_point_norm = !getParam<bool>("disable_picard_residual_norm_check");

  if (!parameters().isParamSetByAddParam("picard_rel_tol"))
    _fixed_point_rel_tol = getParam<Real>("picard_rel_tol");

  if (!parameters().isParamSetByAddParam("picard_abs_tol"))
    _fixed_point_abs_tol = getParam<Real>("picard_abs_tol");

  if (isParamValid("picard_custom_pp"))
    _fixed_point_custom_pp = &getPostprocessorValue("picard_custom_pp");

  if (!parameters().isParamSetByAddParam("picard_force_norms"))
    _fixed_point_force_norms = getParam<bool>("picard_force_norms");

  allocateStorage(true);
}

void
PicardSolve::allocateVariableStorage(SystemBase & system, const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const auto current_app_type = primary ? "_primary_" : "_secondary_";

  const TagID old_tag_id =
      _problem.addVectorTag(system.name() + current_app_type + "xn_m1", Moose::VECTOR_TAG_SOLUTION);

  if (relaxation_factor != 1.)
    // Store a copy of the previous solution
    system.addVector(old_tag_id, false, PARALLEL);
}

void
PicardSolve::allocatePostprocessorStorage(const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const std::vector<PostprocessorName> & transformed_pps =
      primary ? _transformed_pps : _secondary_transformed_pps;
  std::vector<std::vector<PostprocessorValue>> & transformed_pps_values =
      primary ? _transformed_pps_values : _secondary_transformed_pps_values;

  if (relaxation_factor != 1.)
  {
    // Allocate storage for the previous postprocessor values
    transformed_pps_values.resize(transformed_pps.size());
    for (size_t i = 0; i < transformed_pps.size(); i++)
      transformed_pps_values[i].resize(1);
  }
}

void
PicardSolve::saveVariableValues(SystemBase & system, const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const auto current_app_type = primary ? "_primary_" : "_secondary_";
  const TagID old_tag_id = _problem.getVectorTagID(system.name() + current_app_type + "xn_m1");

  if (relaxation_factor != 1.)
  {
    NumericVector<Number> & solution = system.solution();
    NumericVector<Number> & transformed_old = system.getVector(old_tag_id);
    _console << "Saving " << relaxation_factor << " : " << solution(0) << " " << transformed_old(0)
             << std::endl;
    transformed_old = solution;
  }
}

void
PicardSolve::savePostprocessorValues(const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const std::vector<PostprocessorName> & transformed_pps =
      primary ? _transformed_pps : _secondary_transformed_pps;
  std::vector<std::vector<PostprocessorValue>> & transformed_pps_values =
      primary ? _transformed_pps_values : _secondary_transformed_pps_values;

  if (relaxation_factor != 1.)
    // Save postprocessor previous values
    for (size_t i = 0; i < transformed_pps.size(); i++)
      transformed_pps_values[i][0] = getPostprocessorValueByName(transformed_pps[i]);
}

bool
PicardSolve::useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary)
{
  // unrelaxed Picard is the default update for fixed point iterations
  // old values are required for relaxation
  if (primary)
    return _relax_factor != 1. && _fixed_point_it >= _start_fixed_point;
  else
    return _secondary_relaxation_factor != 1. && _main_fixed_point_it > 0;
}

void
PicardSolve::transformPostprocessors(const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const std::vector<PostprocessorName> & transformed_pps =
      primary ? _transformed_pps : _secondary_transformed_pps;
  std::vector<std::vector<PostprocessorValue>> & transformed_pps_values =
      primary ? _transformed_pps_values : _secondary_transformed_pps_values;

  // Relax the postprocessors
  for (size_t i = 0; i < transformed_pps.size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName(transformed_pps[i]);
    const Real old_value = transformed_pps_values[i][0];

    // Compute and set relaxed value
    Real new_value = current_value;
    new_value = relaxation_factor * current_value + (1 - relaxation_factor) * old_value;
    _problem.setPostprocessorValueByName(transformed_pps[i], new_value);
  }
}

void
PicardSolve::transformVariables(SystemBase & system,
                                const std::set<dof_id_type> & transformed_dofs,
                                const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const auto current_app_type = primary ? "_primary_" : "_secondary_";
  const TagID old_tag_id = _problem.getVectorTagID(system.name() + current_app_type + "xn_m1");

  NumericVector<Number> & solution = system.solution();
  NumericVector<Number> & transformed_old = system.getVector(old_tag_id);
  _console << "Relaxing " << relaxation_factor << " : " << solution(0) << " " << transformed_old(0)
           << std::endl;

  for (const auto & dof : transformed_dofs)
    solution.set(dof,
                 (transformed_old(dof) * (1.0 - relaxation_factor)) +
                     (solution(dof) * relaxation_factor));

  solution.close();
  system.update();
}

void
PicardSolve::printFixedPointConvergenceHistory()
{
  _console << "\n 0 Picard |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _fixed_point_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _fixed_point_it; ++i)
  {
    Real max_norm =
        std::max(_fixed_point_timestep_begin_norm[i], _fixed_point_timestep_end_norm[i]);
    _console << std::setw(2) << i + 1
             << " Picard |R| = " << Console::outputNorm(_fixed_point_initial_norm, max_norm)
             << '\n';
  }

  _console << std::endl;
}
