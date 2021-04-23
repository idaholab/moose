//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteffensenSolve.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

InputParameters
SteffensenSolve::validParams()
{
  InputParameters params = FixedPointSolve::validParams();

  return params;
}

SteffensenSolve::SteffensenSolve(Executioner * ex) : FixedPointSolve(ex)
{
  // Store a copy of the state and its first evaluation in the coupled problem
  _problem.getNonlinearSystemBase().addVector("xn_m1", false, PARALLEL);
  _problem.getNonlinearSystemBase().addVector("fxn_m1", false, PARALLEL);

  // Allocate storage for the previous values of the postprocessors to transform
  _transformed_pps_values.resize(_transformed_pps.size());
  for (size_t i = 0; i < _transformed_pps.size(); i++)
    _transformed_pps_values[i].resize(2);

  // Steffensen method uses half-steps
  _min_fixed_point_its *= 2;
  _max_fixed_point_its *= 2;
}

void
SteffensenSolve::allocateStorageForSecondaryTransformed()
{
  // Store a copy of the previous solution here
  _problem.getNonlinearSystemBase().addVector("secondary_xn_m1", false, PARALLEL);
  _problem.getNonlinearSystemBase().addVector("secondary_fxn_m1", false, PARALLEL);

  // Allocate storage for the previous postprocessor values
  _secondary_transformed_pps_values.resize(_secondary_transformed_pps.size());
  for (size_t i = 0; i < _secondary_transformed_pps.size(); i++)
    _secondary_transformed_pps_values[i].resize(2);
}

void
SteffensenSolve::savePreviousVariableValuesAsSubApp()
{
  // Save previous variable values
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector("secondary_fxn_m1");
  NumericVector<Number> & xn_m1 = _nl.getVector("secondary_xn_m1");

  // What 'solution' is with regards to the Steffensen solve depends on the step
  if (_main_fixed_point_it % 2 == 1)
    xn_m1 = solution;
  else
    fxn_m1 = solution;
}

void
SteffensenSolve::savePreviousPostprocessorValuesAsSubApp()
{
  // Save previous postprocessor values
  for (size_t i = 0; i < _secondary_transformed_pps.size(); i++)
  {
    if (_main_fixed_point_it % 2 == 0)
      _secondary_transformed_pps_values[i][1] =
          getPostprocessorValueByName(_secondary_transformed_pps[i]);
    else
      _secondary_transformed_pps_values[i][0] =
          getPostprocessorValueByName(_secondary_transformed_pps[i]);
  }
}

bool
SteffensenSolve::useFixedPointAlgorithmUpdate(bool as_main_app)
{
  // Need at least two values to compute the Steffensen update, and the update is only performed
  // every other iteration as two evaluations of the coupled problem are necessary
  if (as_main_app)
    return _fixed_point_it > 1 && (_fixed_point_it % 2 == 0);
  else
    return _main_fixed_point_it > 1 && (_main_fixed_point_it % 2 == 0);
}

void
SteffensenSolve::savePreviousValuesAsMainApp()
{
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector("fxn_m1");
  NumericVector<Number> & xn_m1 = _nl.getVector("xn_m1");

  // What solution is with regards to the Steffensen solve depends on the step
  if (_fixed_point_it % 2 == 1)
    xn_m1 = solution;
  else
    fxn_m1 = solution;

  // Set postprocessor previous values
  for (size_t i = 0; i < _transformed_pps.size(); i++)
  {
    if (_fixed_point_it % 2 == 0)
      _transformed_pps_values[i][1] = getPostprocessorValueByName(_transformed_pps[i]);
    else
      _transformed_pps_values[i][0] = getPostprocessorValueByName(_transformed_pps[i]);
  }
}

void
SteffensenSolve::transformPostprocessorsAsMainApp()
{
  // Relax postprocessors for the main application
  for (size_t i = 0; i < _transformed_pps.size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName(_transformed_pps[i]);
    const Real fxn_m1 = _transformed_pps_values[i][0];
    const Real xn_m1 = _transformed_pps_values[i][1];

    // Compute and set relaxed value
    Real new_value = current_value;
    if (!MooseUtils::absoluteFuzzyEqual(current_value + xn_m1 - 2 * fxn_m1, 0))
      new_value =
          xn_m1 - (fxn_m1 - xn_m1) * (fxn_m1 - xn_m1) / (current_value + xn_m1 - 2 * fxn_m1);

    // Relax update
    new_value = _relax_factor * new_value + (1 - _relax_factor) * fxn_m1;

    _problem.setPostprocessorValueByName(_transformed_pps[i], new_value);
  }
}

void
SteffensenSolve::transformPostprocessorsAsSubApp()
{
  // Update the postprocessors
  for (size_t i = 0; i < _secondary_transformed_pps.size(); i++)
  {
    // Get new, previous and the one before postprocessor values
    const Real current_value = getPostprocessorValueByName(_secondary_transformed_pps[i]);
    const Real fxn_m1 = _secondary_transformed_pps_values[i][0];
    const Real xn_m1 = _secondary_transformed_pps_values[i][1];

    // Compute the Steffensen method value
    Real new_value = current_value;
    if (!MooseUtils::absoluteFuzzyEqual(current_value + xn_m1 - 2 * fxn_m1, 0))
      new_value =
          xn_m1 - (fxn_m1 - xn_m1) * (fxn_m1 - xn_m1) / (current_value + xn_m1 - 2 * fxn_m1);

    // Relax update
    new_value =
        _secondary_relaxation_factor * new_value + (1 - _secondary_relaxation_factor) * fxn_m1;

    // Update the postprocessor
    _problem.setPostprocessorValueByName(_secondary_transformed_pps[i], new_value);
  }
}

void
SteffensenSolve::transformVariablesAsMainApp(const std::set<dof_id_type> & transformed_dofs)
{
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector("fxn_m1");
  NumericVector<Number> & xn_m1 = _nl.getVector("xn_m1");

  for (const auto & dof : transformed_dofs)
  {
    // Avoid 0 denominator issue
    Real new_value = solution(dof);
    if (!MooseUtils::absoluteFuzzyEqual(solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof), 0))
      new_value = xn_m1(dof) - (fxn_m1(dof) - xn_m1(dof)) * (fxn_m1(dof) - xn_m1(dof)) /
                                   (solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof));

    // Relax update
    new_value = _relax_factor * new_value + (1 - _relax_factor) * fxn_m1(dof);

    solution.set(dof, new_value);
  }
  solution.close();
  _nl.update();
}

void
SteffensenSolve::transformVariablesAsSubApp(
    const std::set<dof_id_type> & secondary_transformed_dofs)
{
  // Update the variables
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector("secondary_fxn_m1");
  NumericVector<Number> & xn_m1 = _nl.getVector("secondary_xn_m1");

  for (const auto & dof : secondary_transformed_dofs)
  {
    // Avoid 0 denominator issue
    Real new_value = solution(dof);
    if (!MooseUtils::absoluteFuzzyEqual(solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof), 0))
      new_value = xn_m1(dof) - (fxn_m1(dof) - xn_m1(dof)) * (fxn_m1(dof) - xn_m1(dof)) /
                                   (solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof));

    // Relax update
    new_value =
        _secondary_relaxation_factor * new_value + (1 - _secondary_relaxation_factor) * fxn_m1(dof);

    solution.set(dof, new_value);
  }
  solution.close();
  _nl.update();
}

void
SteffensenSolve::printFixedPointConvergenceHistory()
{
  _console << "\n 0 Steffensen method    |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _fixed_point_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _fixed_point_it; ++i)
  {
    Real max_norm =
        std::max(_fixed_point_timestep_begin_norm[i], _fixed_point_timestep_end_norm[i]);
    std::stringstream steffensen_prefix;
    if (i % 2 == 0)
      steffensen_prefix << " Steffensen half-step |R| = ";
    else
      steffensen_prefix << " Steffensen step      |R| = ";

    _console << std::setw(2) << i + 1 << steffensen_prefix.str()
             << Console::outputNorm(_fixed_point_initial_norm, max_norm) << '\n';
  }
}
