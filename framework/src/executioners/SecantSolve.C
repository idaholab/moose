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

InputParameters
SecantSolve::validParams()
{
  InputParameters params = FixedPointSolve::validParams();

  return params;
}

SecantSolve::SecantSolve(Executioner & ex) : FixedPointSolve(ex)
{
  allocateVariableStorage(_nl, true);
  // Auxiliary system secant acceleration is not currently enabled
  allocatePostprocessorStorage(true);

  if (parameters().isParamSetByUser("transformed_auxiliary_variables"))
    mooseError("Secant acceleration of auxiliary variables has not been implemented");
}

void
SecantSolve::allocateVariableStorage(SystemBase & system, const bool primary)
{
  const auto & sys_name = system.name();
  const auto current_app_type = primary ? "_primary_" : "_secondary_";
  const auto & solution_tag_type = Moose::VECTOR_TAG_SOLUTION;
  const TagID xn_m1_tagid =
      _problem.addVectorTag(sys_name + current_app_type + "xn_m1", solution_tag_type);
  const TagID fxn_m1_tagid =
      _problem.addVectorTag(sys_name + current_app_type + "_primary_fxn_m1", solution_tag_type);
  const TagID xn_m2_tagid =
      _problem.addVectorTag(sys_name + current_app_type + "_primary_xn_m2", solution_tag_type);
  const TagID fxn_m2_tagid =
      _problem.addVectorTag(sys_name + current_app_type + "_primary_fxn_m2", solution_tag_type);

  // TODO: We would only need to store the solution for the degrees of freedom that
  // will be transformed, not the entire solution.
  // Store solution vectors for the two previous points and their evaluation
  system.addVector(xn_m1_tagid, false, PARALLEL);
  system.addVector(fxn_m1_tagid, false, PARALLEL);
  system.addVector(xn_m2_tagid, false, PARALLEL);
  system.addVector(fxn_m2_tagid, false, PARALLEL);
}

void
SecantSolve::allocatePostprocessorStorage(const bool /* primary */)
{
  _transformed_pps_values.resize(_transformed_pps.size());
  for (size_t i = 0; i < _transformed_pps.size(); i++)
    _transformed_pps_values[i].resize(4);
  _secondary_transformed_pps_values.resize(_secondary_transformed_pps.size());
  for (size_t i = 0; i < _secondary_transformed_pps.size(); i++)
    _secondary_transformed_pps_values[i].resize(4);
}

void
SecantSolve::saveVariableValues(SystemBase & system, const bool primary)
{
  const auto current_app_type = primary ? "_primary_" : "_secondary_";
  const TagID fxn_m1_tagid = _problem.getVectorTagID(system.name() + current_app_type + "fxn_m1");
  const TagID xn_m1_tagid = _problem.getVectorTagID(system.name() + current_app_type + "xn_m1");
  const TagID fxn_m2_tagid = _problem.getVectorTagID(system.name() + current_app_type + "fxn_m2");
  const TagID xn_m2_tagid = _problem.getVectorTagID(system.name() + current_app_type + "xn_m2");

  // Save previous variable values
  NumericVector<Number> & solution = system.solution();
  NumericVector<Number> & fxn_m1 = system.getVector(fxn_m1_tagid);
  NumericVector<Number> & xn_m1 = system.getVector(xn_m1_tagid);
  NumericVector<Number> & fxn_m2 = system.getVector(fxn_m2_tagid);
  NumericVector<Number> & xn_m2 = system.getVector(xn_m2_tagid);

  // Advance one step
  xn_m2 = xn_m1;
  fxn_m2 = fxn_m1;

  // Before a solve, solution is a sequence term, after a solve, solution is the evaluated term
  xn_m1 = solution;
}

void
SecantSolve::savePostprocessorValues(const bool primary)
{
  const std::vector<PostprocessorName> & transformed_pps =
      primary ? _transformed_pps : _secondary_transformed_pps;
  const std::vector<std::vector<PostprocessorValue>> & transformed_pps_values =
      primary ? _transformed_pps_values : _secondary_transformed_pps_values;

  // Save previous postprocessor values
  for (size_t i = 0; i < transformed_pps.size(); i++)
  {
    // Advance one step
    transformed_pps_values[i][3] = transformed_pps_values[i][1];
    transformed_pps_values[i][2] = transformed_pps_values[i][0];

    // Save current value
    // Primary: this is done before the timestep's solves and before timestep_begin transfers,
    // so the value is the result of the previous Secant update (xn_m1)
    // Secondary: this is done after the secondary solve, but before timestep_end postprocessors
    // are computed, or timestep_end transfers are received.
    // This value is the same as before the solve (xn_m1)
    transformed_pps_values[i][1] = getPostprocessorValueByName(transformed_pps[i]);
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
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const std::vector<PostprocessorName> & transformed_pps =
      primary ? _transformed_pps : _secondary_transformed_pps;
  const std::vector<std::vector<PostprocessorValue>> & transformed_pps_values =
      primary ? _transformed_pps_values : _secondary_transformed_pps_values;

  // Relax postprocessors for the main application
  for (size_t i = 0; i < transformed_pps.size(); i++)
  {
    // Get new postprocessor value
    const Real fxn_m1 = getPostprocessorValueByName(transformed_pps[i]);
    const Real xn_m1 = transformed_pps_values[i][1];
    const Real fxn_m2 = transformed_pps_values[i][2];
    const Real xn_m2 = transformed_pps_values[i][3];

    // Save fxn_m1, received or computed before the solve
    transformed_pps_values[i][0] = fxn_m1;

    // Compute and set relaxed value
    Real new_value = fxn_m1;
    if (!MooseUtils::absoluteFuzzyEqual(fxn_m1 - xn_m1 - fxn_m2 + xn_m2, 0))
      new_value = xn_m1 - (fxn_m1 - xn_m1) * (xn_m1 - xn_m2) / (fxn_m1 - xn_m1 - fxn_m2 + xn_m2);

    // Relax update if desired
    new_value = relaxation_factor * new_value + (1 - relaxation_factor) * xn_m1;

    _problem.setPostprocessorValueByName(transformed_pps[i], new_value);
  }
}

void
SecantSolve::transformVariables(SystemBase & system,
                                const std::set<dof_id_type> & target_dofs,
                                const bool primary)
{
  const Real relaxation_factor = primary ? _relax_factor : _secondary_relaxation_factor;
  const auto current_app_type = primary ? "_primary_" : "_secondary_";
  const TagID fxn_m1_tagid = _problem.getVectorTagID(system.name() + current_app_type + "fxn_m1");
  const TagID xn_m1_tagid = _problem.getVectorTagID(system.name() + current_app_type + "xn_m1");
  const TagID fxn_m2_tagid = _problem.getVectorTagID(system.name() + current_app_type + "fxn_m2");
  const TagID xn_m2_tagid = _problem.getVectorTagID(system.name() + current_app_type + "xn_m2");

  NumericVector<Number> & solution = system.solution();
  NumericVector<Number> & xn_m1 = system.getVector(xn_m1_tagid);
  NumericVector<Number> & fxn_m2 = system.getVector(fxn_m2_tagid);
  NumericVector<Number> & xn_m2 = system.getVector(xn_m2_tagid);

  // Save the most recent evaluation of the coupled problem
  NumericVector<Number> & fxn_m1 = system.getVector(fxn_m1_tagid);
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
  system.update();
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
    if (i < 2)
      secant_prefix << " Secant initialization |R| = ";
    else
      secant_prefix << " Secant step           |R| = ";

    _console << std::setw(2) << i + 1 << secant_prefix.str()
             << Console::outputNorm(_fixed_point_initial_norm, max_norm) << '\n';
  }
}
