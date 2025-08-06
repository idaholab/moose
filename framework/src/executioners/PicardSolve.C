//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  return params;
}

PicardSolve::PicardSolve(Executioner & ex) : FixedPointSolve(ex)
{
  allocateStorage(true);

  if (performingRelaxation(true))
    _solver_sys.needSolutionState(1, Moose::SolutionIterationType::FixedPoint);
}

void
PicardSolve::allocateStorage(const bool primary)
{
  if (!performingRelaxation(primary))
    return;

  TagID old_tag_id;
  const std::vector<PostprocessorName> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    old_tag_id = _problem.addVectorTag(Moose::PREVIOUS_FP_SOLUTION_TAG, Moose::VECTOR_TAG_SOLUTION);
    _old_tag_id = old_tag_id;
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
  }
  else
  {
    old_tag_id = _problem.addVectorTag("secondary_xn_m1", Moose::VECTOR_TAG_SOLUTION);
    _secondary_old_tag_id = old_tag_id;
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
  }

  // Store a copy of the previous solution
  _solver_sys.addVector(old_tag_id, false, PARALLEL);

  // Allocate storage for the previous postprocessor values
  (*transformed_pps_values).resize((*transformed_pps).size());
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
    (*transformed_pps_values)[i].resize(1);
}

void
PicardSolve::saveVariableValues(const bool primary)
{
  // Primary is copied back by _solver_sys.copyPreviousFixedPointSolutions()
  if (!performingRelaxation(primary) || primary)
    return;

  // Check to make sure allocateStorage has been called
  mooseAssert(_secondary_old_tag_id != Moose::INVALID_TAG_ID,
              "allocateStorage has not been called with primary = " + Moose::stringify(primary));

  // Save variable previous values
  NumericVector<Number> & solution = _solver_sys.solution();
  NumericVector<Number> & transformed_old = _solver_sys.getVector(_secondary_old_tag_id);
  transformed_old = solution;
}

void
PicardSolve::savePostprocessorValues(const bool primary)
{
  if (!performingRelaxation(primary))
    return;

  const std::vector<PostprocessorName> * transformed_pps;
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

  // Save postprocessor previous values
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
    (*transformed_pps_values)[i][0] = getPostprocessorValueByName((*transformed_pps)[i]);
}

bool
PicardSolve::useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary)
{
  // unrelaxed Picard is the default update for fixed point iterations
  // old values are required for relaxation
  const auto fixed_point_it = primary ? _fixed_point_it : _main_fixed_point_it;
  return performingRelaxation(primary) && fixed_point_it > 0;
}

void
PicardSolve::transformPostprocessors(const bool primary)
{
  Real relaxation_factor;
  const std::vector<PostprocessorName> * transformed_pps;
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

  // Relax the postprocessors
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName((*transformed_pps)[i]);
    const Real old_value = (*transformed_pps_values)[i][0];

    // Compute and set relaxed value
    Real new_value = current_value;
    new_value = relaxation_factor * current_value + (1 - relaxation_factor) * old_value;
    _problem.setPostprocessorValueByName((*transformed_pps)[i], new_value);
  }
}

void
PicardSolve::transformVariables(const std::set<dof_id_type> & transformed_dofs, const bool primary)
{
  Real relaxation_factor;
  TagID old_tag_id;
  if (primary)
  {
    relaxation_factor = _relax_factor;
    old_tag_id = _old_tag_id;
  }
  else
  {
    relaxation_factor = _secondary_relaxation_factor;
    old_tag_id = _secondary_old_tag_id;
  }

  NumericVector<Number> & solution = _solver_sys.solution();
  NumericVector<Number> & transformed_old = _solver_sys.getVector(old_tag_id);

  for (const auto & dof : transformed_dofs)
    solution.set(dof,
                 (transformed_old(dof) * (1.0 - relaxation_factor)) +
                     (solution(dof) * relaxation_factor));

  solution.close();
  _solver_sys.update();
}

void
PicardSolve::printFixedPointConvergenceHistory(Real initial_norm,
                                               const std::vector<Real> & timestep_begin_norms,
                                               const std::vector<Real> & timestep_end_norms) const
{
  _console << "\n 0 Picard |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), initial_norm) << '\n';

  Real max_norm_old = initial_norm;
  for (unsigned int i = 0; i <= _fixed_point_it; ++i)
  {
    Real max_norm = std::max(timestep_begin_norms[i], timestep_end_norms[i]);
    _console << std::setw(2) << i + 1
             << " Picard |R| = " << Console::outputNorm(max_norm_old, max_norm) << '\n';
    max_norm_old = max_norm;
  }

  _console << std::endl;
}
