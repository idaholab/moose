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
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

defineLegacyParams(PicardSolve);

InputParameters
PicardSolve::validParams()
{
  InputParameters params = IterativeMultiAppSolve::validParams();

  params.addDeprecatedParam<unsigned int>("picard_max_its",
                                          1,
                                          "Deprecated, use coupling_max_its",
                                          "Specifies the maximum number of Picard iterations. "
                                          "Mainly used when  wanting to do Picard iterations with MultiApps "
                                          "that are set to execute_on timestep_end or timestep_begin. "
                                          "Setting this parameter to 1 turns off the Picard iterations.");
  params.addDeprecatedParam<bool>(
      "accept_on_max_picard_iteration",
      false,
      "Deprecated, use accept_on_max_coupling_iteration",
      "True to treat reaching the maximum number of Picard iterations as converged.");
  params.addDeprecatedParam<bool>("disable_picard_residual_norm_check",
                                  false,
                                  "Deprecated, use disable_coupling_residual_norm_check",
                                  "Disable the Picard residual norm evaluation thus the three parameters "
                                  "picard_rel_tol, picard_abs_tol and picard_force_norms.");
  params.addDeprecatedParam<Real>("picard_rel_tol",
                                  1e-8,
                                  "Deprecated, use coupling_rel_tol",
                                  "The relative nonlinear residual drop to shoot for "
                                  "during Picard iterations. This check is "
                                  "performed based on the Master app's nonlinear "
                        "residual.");
  params.addDeprecatedParam<Real>("picard_abs_tol",
                                  1e-50,
                                  "Deprecated, use coupling_abs_tol",
                                  "The absolute nonlinear residual to shoot for "
                                  "during Picard iterations. This check is "
                                  "performed based on the Master app's nonlinear "
                                  "residual.");
  params.addDeprecatedParam<PostprocessorName>("picard_custom_pp",
                                    "Deprecated, use coupling_custom_pp",
                                    "Postprocessor for custom picard convergence check.");

  params.addDeprecatedParam<bool>(
      "picard_force_norms",
      false,
      "Deprecated, use coupling_force_norms",
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existence of active MultiApps with those execute_on flags, default: false.");

  return params;
}

PicardSolve::PicardSolve(Executioner * ex)
  : IterativeMultiAppSolve(ex)
{
  // Handle deprecated parameters
  if (!parameters().isParamSetByAddParam("picard_max_its"))
  {
    _coupling_max_its = getParam<unsigned int>("picard_max_its");
    _has_coupling_its = _coupling_max_its > 1;
  }

  if (!parameters().isParamSetByAddParam("accept_on_max_picard_iteration"))
    _accept_max_it = getParam<bool>("accept_on_max_picard_iteration");

  if (!parameters().isParamSetByAddParam("disable_picard_residual_norm_check"))
    _has_coupling_norm = !getParam<bool>("disable_picard_residual_norm_check");

  if (!parameters().isParamSetByAddParam("picard_rel_tol"))
    _coupling_rel_tol = getParam<Real>("picard_rel_tol");

  if (!parameters().isParamSetByAddParam("picard_abs_tol"))
    _coupling_abs_tol = getParam<Real>("picard_abs_tol");

  if (isParamValid("picard_custom_pp"))
    _coupling_custom_pp = &getPostprocessorValue("picard_custom_pp");

  if (!parameters().isParamSetByAddParam("picard_force_norms"))
    _coupling_force_norms = getParam<bool>("picard_force_norms");

  // Resize array for old values of postprocessors
  _old_transformed_pps_values.resize(_transformed_pps.size());
}

void
PicardSolve::savePreviousValuesAsSubApp()
{
  if (_secondary_relaxation_factor != 1.)
  {
    // Save variable previous values
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & transformed_old = _nl.getVector("secondary_transformed_old");
    transformed_old = solution;

    // Set postprocessor previous values
    for (size_t i=0; i<_secondary_transformed_pps.size(); i++)
      _old_secondary_transformed_pps_values[i] = getPostprocessorValueByName(_secondary_transformed_pps[i]);
  }
}

void PicardSolve::savePreviousValuesAsMainApp()
{
  if (_relax_factor != 1)
  {
    // Save variable previous values
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & transformed_old = _nl.getVector("transformed_old");
    transformed_old = solution;

    // Set postprocessor previous values
    for (size_t i=0; i<_transformed_pps.size(); i++)
      _old_transformed_pps_values[i] = getPostprocessorValueByName(_transformed_pps[i]);
  }
}

bool
PicardSolve::useCouplingUpdateAlgorithm()
{
  // unrelaxed Picard is the default update for multiapp coupling
  // old values are required for relaxation
  return _relax_factor != 1. && _coupling_it > 0;/////////////////////////////////
}

void
PicardSolve::updatePostprocessorsAsMainApp()
{
  if (_relax_factor != 1.)
  {
    // Relax the postprocessors
    std::cout << "pp main relaxing iteration: " << _coupling_it << std::endl;
    for (size_t i=0; i<_transformed_pps.size(); i++)
    {
      // Get new postprocessor value
      const Real current_value = getPostprocessorValueByName(_transformed_pps[i]);
      const Real old_value = _old_transformed_pps_values[i];

      // Compute and set relaxed value
      Real new_value = current_value;
      const Real factor = _relax_factor;

      if (_has_old_pp_values)
        new_value = factor * current_value + (1 - factor) * old_value;
      _problem.setPostprocessorValueByName(_transformed_pps[i], new_value);
      // _old_transformed_pps_values[i] = current_value;

      std::cout << _transformed_pps[i] << " " << current_value << " & " << old_value  << " -> " << new_value << std::endl;
    }
  }
}

void
PicardSolve::updateVariablesAsMainApp(const std::set<dof_id_type> & transformed_dofs)
{
  if (_relax_factor != 1.)
  {
    std::cout << "Relaxing IN STEP " << std::endl;
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & transformed_old = _nl.getVector("transformed_old");
    const Real factor = _relax_factor;

    for (const auto & dof : transformed_dofs)
      solution.set(dof, (transformed_old(dof) * (1.0 - factor)) + (solution(dof) * factor));

    solution.close();
    _nl.update();
  }
}

void
PicardSolve::updateAsSubApp(const std::set<dof_id_type> & secondary_transformed_dofs)
{
  if (_secondary_relaxation_factor != 1.)
  {
    std::cout << "RELAXING POST coupling" << std::endl;
    if (_old_entering_time == _problem.time())
    {
      NumericVector<Number> & solution = _nl.solution();
      NumericVector<Number> & transformed_old = _nl.getVector("secondary_transformed_old");
      const Real factor = _secondary_relaxation_factor;

      for (const auto & dof : secondary_transformed_dofs)
        solution.set(dof, (transformed_old(dof) * (1.0 - factor)) + (solution(dof) * factor));
      solution.close();
      _nl.update();


      // Relax the postprocessors
      std::cout << "Sub-coupling iteration: " << _coupling_it << std::endl;
      for (size_t i=0; i<_secondary_transformed_pps.size(); i++)
      {
        // Get new postprocessor value
        const Real current_value = getPostprocessorValueByName(_secondary_transformed_pps[i]);
        const Real old_value = _old_secondary_transformed_pps_values[i];

        // Compute and set relaxed value
        Real new_value = current_value;
        const Real factor = _secondary_relaxation_factor;

        if (_has_old_pp_values)
          new_value = factor * current_value + (1 - factor) * old_value;
        _problem.setPostprocessorValueByName(_secondary_transformed_pps[i], new_value);
        // _old_secondary_transformed_pps_values[i] = current_value;

        std::cout << _secondary_transformed_pps[i] << " " << current_value << " & " << old_value  << " -> " << new_value << std::endl;
      }
      _has_old_pp_values = true;
    }
  }
}

void
PicardSolve::printCouplingConvergenceHistory()
{
  _console << "\n 0 Picard |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _coupling_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _coupling_it; ++i)
  {
    Real max_norm = std::max(_coupling_timestep_begin_norm[i], _coupling_timestep_end_norm[i]);
    _console << std::setw(2) << i + 1
             << " Picard |R| = " << Console::outputNorm(_coupling_initial_norm, max_norm)
             << '\n';
  }
}
