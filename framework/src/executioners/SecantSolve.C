//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecantSolve.h"

#include "Executioner.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

defineLegacyParams(SecantSolve);

InputParameters
SecantSolve::validParams()
{
  InputParameters params = IterativeMultiAppSolve::validParams();

  return params;
}

SecantSolve::SecantSolve(Executioner * ex)
  : IterativeMultiAppSolve(ex)
{
  // Store a copy of the solution before the previous solution here
  _problem.getNonlinearSystemBase().addVector("transformed_older", false, PARALLEL);

  _old_transformed_pps_values.resize(_transformed_pps.size());
  _older_transformed_pps_values.resize(_transformed_pps.size());
}

void SecantSolve::savePreviousValuesAsSubApp()
{
  // Save previous variable values
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & transformed_old = _nl.getVector("secondary_transformed_old");
  NumericVector<Number> & transformed_older = _nl.getVector("secondary_transformed_older");
  transformed_older = transformed_old;
  transformed_old = solution;

  // Save previous postprocessor values
  for (size_t i=0; i<_secondary_transformed_pps.size(); i++)
  {
    _older_secondary_transformed_pps_values[i] = _old_secondary_transformed_pps_values[i];
    _old_secondary_transformed_pps_values[i] = getPostprocessorValueByName(_secondary_transformed_pps[i]);
  }
}

void SecantSolve::savePreviousValuesAsMainApp()
{
  // Save previous variable values
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & transformed_old = _nl.getVector("transformed_old");
  NumericVector<Number> & transformed_older = _nl.getVector("transformed_older");

  // Save off the current and previous solutions
  transformed_older = transformed_old;
  transformed_old = solution;

  // Set postprocessor previous values
  for (size_t i=0; i<_transformed_pps.size(); i++)
  {
    _older_transformed_pps_values[i] = _old_transformed_pps_values[i];
    _old_transformed_pps_values[i] = getPostprocessorValueByName(_transformed_pps[i]);
  }
}


bool SecantSolve::useCouplingUpdateAlgorithm()
{
  // Need at least two values to compute the Secant slope
  return _coupling_it > 1;
}

void SecantSolve::updatePostprocessorsAsMainApp()
{
  // Relax postprocessors for the main application
  std::cout << "Secant method iteration: " << _coupling_it << std::endl;
  for (size_t i=0; i<_transformed_pps.size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName(_transformed_pps[i]);
    const Real old_value = _old_transformed_pps_values[i];
    const Real older_value = _older_transformed_pps_values[i];

    // Compute and set relaxed value
    Real new_value;
    if (!MooseUtils::absoluteFuzzyEqual(current_value + older_value - 2*old_value, 0))
      new_value = old_value - (current_value - old_value) * (old_value - older_value) /
          (current_value - 2*old_value + older_value);
    else
      new_value = current_value;

    _problem.setPostprocessorValueByName(_transformed_pps[i], new_value);

    // Print new value
    std::cout << _transformed_pps[i] << " " << current_value << " & " << old_value << " & " << older_value << " -> " << new_value << std::endl;
  }
}

void SecantSolve::updateVariablesAsMainApp(const std::set<dof_id_type> & target_dofs)
{
  std::cout << "Relaxing IN STEP " << std::endl;
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & transformed_old = _nl.getVector("transformed_old");
  NumericVector<Number> & transformed_older = _nl.getVector("transformed_older");

  for (const auto & dof : target_dofs)
  {
    // Avoid 0 denominator issue
    if (!MooseUtils::absoluteFuzzyEqual(solution(dof) + transformed_older(dof) - 2*transformed_old(dof) , 0))
      solution.set(dof, transformed_older(dof) - (transformed_old(dof) - transformed_older(dof)) * (transformed_old(dof) - transformed_older(dof))
                   / (solution(dof) + transformed_older(dof) - 2*transformed_old(dof)));
    else
      solution.set(dof, solution(dof));
  }
  solution.close();
  _nl.update();
}

void SecantSolve::updateAsSubApp(const std::set<dof_id_type> & secondary_transformed_dofs)
{
  std::cout << "RELAXING POST SECANT" << std::endl;
  if (_old_entering_time == _problem.time())
  {
    // Update the variables
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & transformed_old = _nl.getVector("secondary_transformed_old");
    NumericVector<Number> & transformed_older = _nl.getVector("secondary_transformed_older");

    for (const auto & dof : secondary_transformed_dofs)\
    {
      // Avoid 0 denominator issue
      if (!MooseUtils::absoluteFuzzyEqual(solution(dof) + transformed_older(dof) - 2*transformed_old(dof) , 0))
      solution.set(dof, transformed_old(dof) - (solution(dof) - transformed_old(dof)) * (transformed_old(dof) - transformed_older(dof))
          / (solution(dof) + transformed_older(dof) - 2*transformed_old(dof)));
      else
        solution.set(dof, solution(dof));
    }
    solution.close();
    _nl.update();

    // Update the postprocessors
    std::cout << "Sub-Secant method iteration: " << _coupling_it << std::endl;
    for (size_t i=0; i<_secondary_transformed_pps.size(); i++)
    {
      // Get new, previous and the one before postprocessor values
      const Real current_value = getPostprocessorValueByName(_secondary_transformed_pps[i]);
      const Real old_value = _old_secondary_transformed_pps_values[i];
      const Real older_value = _older_secondary_transformed_pps_values[i];

      // Compute the Secant method value
      Real new_value = current_value;
      if (!MooseUtils::absoluteFuzzyEqual(current_value + older_value - 2 * old_value, 0))
        new_value = old_value - (current_value - old_value) * (old_value - older_value) /
            (current_value + older_value - 2 * old_value);
      else
        new_value = current_value;

      // Update the postprocessor
      _problem.setPostprocessorValueByName(_secondary_transformed_pps[i], new_value);

      std::cout << _secondary_transformed_pps[i] << " " << current_value << " & " << old_value  << " " << _problem.getPostprocessorValueByName(_secondary_transformed_pps[i], 1) << " -> " << new_value << std::endl;
    }
  }
  _old_entering_time = _problem.time();
}

void SecantSolve::printCouplingConvergenceHistory()
{
  _console << "\n 0 Secant method    |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _coupling_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _coupling_it; ++i)
  {
    Real max_norm = std::max(_coupling_timestep_begin_norm[i], _coupling_timestep_end_norm[i]);
    std::stringstream secant_prefix;
    secant_prefix << " Secant step      |R| = ";

    _console << std::setw(2) << i + 1
             << secant_prefix.str() << Console::outputNorm(_coupling_initial_norm, max_norm)
             << '\n';
  }
}
