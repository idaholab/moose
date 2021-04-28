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
#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

defineLegacyParams(SecantSolve);

InputParameters
SecantSolve::validParams()
{
  InputParameters params = FixedPointSolve::validParams();

  return params;
}

SecantSolve::SecantSolve(Executioner * ex) : FixedPointSolve(ex) { allocateStorage(true); }

void
SecantSolve::allocateStorage(const bool primary)
{
  std::string fxn_m1_name;
  std::string xn_m1_name;
  std::string fxn_m2_name;
  std::string xn_m2_name;
  const std::vector<std::string> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    fxn_m1_name = "fxn_m1";
    xn_m1_name = "xn_m1";
    fxn_m2_name = "fxn_m2";
    xn_m2_name = "xn_m2";
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
  }
  else
  {
    fxn_m1_name = "secondary_fxn_m1";
    xn_m1_name = "secondary_xn_m1";
    fxn_m2_name = "secondary_fxn_m2";
    xn_m2_name = "secondary_xn_m2";
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
  }

  // TODO: We would only need to store the solution for the degrees of freedom that
  // will be transformed, not the entire solution.
  // Store solution vectors for the two previous points and their evaluation
  _problem.getNonlinearSystemBase().addVector(xn_m1_name, false, PARALLEL);
  _problem.getNonlinearSystemBase().addVector(fxn_m1_name, false, PARALLEL);
  _problem.getNonlinearSystemBase().addVector(xn_m2_name, false, PARALLEL);
  _problem.getNonlinearSystemBase().addVector(fxn_m2_name, false, PARALLEL);

  // Allocate storage for the previous values of the postprocessors to transform
  (*transformed_pps_values).resize((*transformed_pps).size());
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
    (*transformed_pps_values)[i].resize(4);
}

void
SecantSolve::saveVariableValues(const bool primary)
{
  std::string fxn_m1_name;
  std::string xn_m1_name;
  std::string fxn_m2_name;
  std::string xn_m2_name;
  if (primary)
  {
    fxn_m1_name = "fxn_m1";
    xn_m1_name = "xn_m1";
    fxn_m2_name = "fxn_m2";
    xn_m2_name = "xn_m2";
  }
  else
  {
    fxn_m1_name = "secondary_fxn_m1";
    xn_m1_name = "secondary_xn_m1";
    fxn_m2_name = "secondary_fxn_m2";
    xn_m2_name = "secondary_xn_m2";
  }

  // Save previous variable values
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector(fxn_m1_name);
  NumericVector<Number> & xn_m1 = _nl.getVector(xn_m1_name);
  NumericVector<Number> & fxn_m2 = _nl.getVector(fxn_m2_name);
  NumericVector<Number> & xn_m2 = _nl.getVector(xn_m2_name);

  // Advance one step
  xn_m2 = xn_m1;
  fxn_m2 = fxn_m1;

  // Before a solve, solution is a sequence term, after a solve, solution is the evaluated term
  xn_m1 = solution;
}

void
SecantSolve::savePostprocessorValues(const bool primary)
{
  const std::vector<std::string> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
  }
  else
  {
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
  }

  // Save previous postprocessor values
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
  {
    // Advance one step
    (*transformed_pps_values)[i][3] = (*transformed_pps_values)[i][1];
    (*transformed_pps_values)[i][2] = (*transformed_pps_values)[i][0];

    // Secondary postprocessors are saved after a solve, but transfers from the main app have not
    // been received yet
    (*transformed_pps_values)[i][1] = getPostprocessorValueByName((*transformed_pps)[i]);
  }
}

bool
SecantSolve::useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary)
{
  // Need at least two evaluations to compute the Secant slope
  if (primary)
    return _fixed_point_it > 1;
  else
    return _main_fixed_point_it > 1;
}

void
SecantSolve::transformPostprocessors(const bool primary)
{
  Real relaxation_factor;
  const std::vector<std::string> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    relaxation_factor = _relax_factor;
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
  }
  else
  {
    relaxation_factor = _secondary_relaxation_factor;
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
  }

  // Relax postprocessors for the main application
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName((*transformed_pps)[i]);
    const Real xn_m1 = (*transformed_pps_values)[i][1];
    const Real fxn_m2 = (*transformed_pps_values)[i][2];
    const Real xn_m2 = (*transformed_pps_values)[i][3];

    // Save fxn_m1, received or computed before the solve
    (*transformed_pps_values)[i][0] = current_value;

    // Compute and set relaxed value
    Real new_value = current_value;
    if (!MooseUtils::absoluteFuzzyEqual(current_value - xn_m1 - fxn_m2 + xn_m2, 0))
      new_value = xn_m1 - (current_value - xn_m1) * (xn_m1 - xn_m2) /
                              (current_value - xn_m1 - fxn_m2 + xn_m2);

    // Relax update if desired
    new_value = relaxation_factor * new_value + (1 - relaxation_factor) * xn_m1;

    _problem.setPostprocessorValueByName((*transformed_pps)[i], new_value);
  }
}

void
SecantSolve::transformVariables(const std::set<dof_id_type> & target_dofs, const bool primary)
{
  Real relaxation_factor;
  std::string fxn_m1_name;
  std::string xn_m1_name;
  std::string fxn_m2_name;
  std::string xn_m2_name;
  if (primary)
  {
    relaxation_factor = _relax_factor;
    fxn_m1_name = "fxn_m1";
    xn_m1_name = "xn_m1";
    fxn_m2_name = "fxn_m2";
    xn_m2_name = "xn_m2";
  }
  else
  {
    relaxation_factor = _secondary_relaxation_factor;
    fxn_m1_name = "secondary_fxn_m1";
    xn_m1_name = "secondary_xn_m1";
    fxn_m2_name = "secondary_fxn_m2";
    xn_m2_name = "secondary_xn_m2";
  }

  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & xn_m1 = _nl.getVector(xn_m1_name);
  NumericVector<Number> & fxn_m2 = _nl.getVector(fxn_m2_name);
  NumericVector<Number> & xn_m2 = _nl.getVector(xn_m2_name);

  // Save the most recent evaluation of the coupled problem
  NumericVector<Number> & fxn_m1 = _nl.getVector(fxn_m1_name);
  fxn_m1 = solution;

  for (const auto & dof : target_dofs)
  {
    // Avoid 0 denominator issue
    Real new_value = fxn_m1(dof);
    if (!MooseUtils::absoluteFuzzyEqual(solution(dof) - xn_m1(dof) - fxn_m2(dof) + xn_m2(dof), 0))
      new_value = xn_m1(dof) - (solution(dof) - xn_m1(dof)) * (xn_m1(dof) - xn_m2(dof)) /
                                   (solution(dof) - xn_m1(dof) - fxn_m2(dof) + xn_m2(dof));

    // Relax update
    new_value = relaxation_factor * new_value + (1 - relaxation_factor) * xn_m1(dof);

    solution.set(dof, new_value);
  }
  solution.close();
  _nl.update();
}

void
SecantSolve::printFixedPointConvergenceHistory()
{
  _console << "\n 0 Secant initialization |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _fixed_point_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _fixed_point_it; ++i)
  {
    Real max_norm =
        std::max(_fixed_point_timestep_begin_norm[i], _fixed_point_timestep_end_norm[i]);
    std::stringstream secant_prefix;
    if (i < 1)
      secant_prefix << " Secant initialization |R| = ";
    else
      secant_prefix << " Secant step           |R| = ";

    _console << std::setw(2) << i + 1 << secant_prefix.str()
             << Console::outputNorm(_fixed_point_initial_norm, max_norm) << '\n';
  }
}
