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

SteffensenSolve::SteffensenSolve(Executioner & ex) : FixedPointSolve(ex)
{
  allocateStorage(true);

  // Steffensen method uses half-steps
  if (!parameters().isParamSetByAddParam("fixed_point_min_its"))
    _min_fixed_point_its *= 2;
  _max_fixed_point_its *= 2;
}

void
SteffensenSolve::allocateStorage(const bool primary)
{
  TagID fxn_m1_tagid;
  TagID xn_m1_tagid;
  const std::vector<PostprocessorName> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    xn_m1_tagid = _problem.addVectorTag("xn_m1", Moose::VECTOR_TAG_SOLUTION);
    fxn_m1_tagid = _problem.addVectorTag("fxn_m1", Moose::VECTOR_TAG_SOLUTION);
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
    _xn_m1_tagid = xn_m1_tagid;
    _fxn_m1_tagid = fxn_m1_tagid;
  }
  else
  {
    xn_m1_tagid = _problem.addVectorTag("secondary_xn_m1", Moose::VECTOR_TAG_SOLUTION);
    fxn_m1_tagid = _problem.addVectorTag("secondary_fxn_m1", Moose::VECTOR_TAG_SOLUTION);
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
    _secondary_xn_m1_tagid = xn_m1_tagid;
    _secondary_fxn_m1_tagid = fxn_m1_tagid;
  }

  // Store a copy of the previous solution here
  _nl.addVector(xn_m1_tagid, false, PARALLEL);
  _nl.addVector(fxn_m1_tagid, false, PARALLEL);

  // Allocate storage for the previous postprocessor values
  (*transformed_pps_values).resize((*transformed_pps).size());
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
    (*transformed_pps_values)[i].resize(2);
}

void
SteffensenSolve::saveVariableValues(const bool primary)
{
  unsigned int iteration;
  TagID fxn_m1_tagid;
  TagID xn_m1_tagid;
  if (primary)
  {
    iteration = _fixed_point_it;
    fxn_m1_tagid = _fxn_m1_tagid;
    xn_m1_tagid = _xn_m1_tagid;
  }
  else
  {
    iteration = _main_fixed_point_it;
    fxn_m1_tagid = _secondary_fxn_m1_tagid;
    xn_m1_tagid = _secondary_xn_m1_tagid;
  }

  // Save previous variable values
  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector(fxn_m1_tagid);
  NumericVector<Number> & xn_m1 = _nl.getVector(xn_m1_tagid);

  // What 'solution' is with regards to the Steffensen solve depends on the step
  if (iteration % 2 == 1)
    xn_m1 = solution;
  else
    fxn_m1 = solution;
}

void
SteffensenSolve::savePostprocessorValues(const bool primary)
{
  unsigned int iteration;
  const std::vector<PostprocessorName> * transformed_pps;
  std::vector<std::vector<PostprocessorValue>> * transformed_pps_values;
  if (primary)
  {
    iteration = _fixed_point_it;
    transformed_pps = &_transformed_pps;
    transformed_pps_values = &_transformed_pps_values;
  }
  else
  {
    iteration = _main_fixed_point_it;
    transformed_pps = &_secondary_transformed_pps;
    transformed_pps_values = &_secondary_transformed_pps_values;
  }

  // Save previous postprocessor values
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
  {
    if (iteration % 2 == 0)
      (*transformed_pps_values)[i][1] = getPostprocessorValueByName((*transformed_pps)[i]);
    else
      (*transformed_pps_values)[i][0] = getPostprocessorValueByName((*transformed_pps)[i]);
  }
}

bool
SteffensenSolve::useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary)
{
  // Need at least two values to compute the Steffensen update, and the update is only performed
  // every other iteration as two evaluations of the coupled problem are necessary
  if (primary)
    return _fixed_point_it > 1 && (_fixed_point_it % 2 == 0);
  else
    return _main_fixed_point_it > 1 && (_main_fixed_point_it % 2 == 0);
}

void
SteffensenSolve::transformPostprocessors(const bool primary)
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

  // Relax postprocessors for the main application
  for (size_t i = 0; i < (*transformed_pps).size(); i++)
  {
    // Get new postprocessor value
    const Real current_value = getPostprocessorValueByName((*transformed_pps)[i]);
    const Real fxn_m1 = (*transformed_pps_values)[i][0];
    const Real xn_m1 = (*transformed_pps_values)[i][1];

    // Compute and set relaxed value
    Real new_value = current_value;
    if (!MooseUtils::absoluteFuzzyEqual(current_value + xn_m1 - 2 * fxn_m1, 0))
      new_value =
          xn_m1 - (fxn_m1 - xn_m1) * (fxn_m1 - xn_m1) / (current_value + xn_m1 - 2 * fxn_m1);

    // Relax update
    new_value = relaxation_factor * new_value + (1 - relaxation_factor) * fxn_m1;

    _problem.setPostprocessorValueByName((*transformed_pps)[i], new_value);
  }
}

void
SteffensenSolve::transformVariables(const std::set<dof_id_type> & transformed_dofs,
                                    const bool primary)
{
  Real relaxation_factor;
  TagID fxn_m1_tagid;
  TagID xn_m1_tagid;
  if (primary)
  {
    relaxation_factor = _relax_factor;
    fxn_m1_tagid = _fxn_m1_tagid;
    xn_m1_tagid = _xn_m1_tagid;
  }
  else
  {
    relaxation_factor = _secondary_relaxation_factor;
    fxn_m1_tagid = _secondary_fxn_m1_tagid;
    xn_m1_tagid = _secondary_xn_m1_tagid;
  }

  NumericVector<Number> & solution = _nl.solution();
  NumericVector<Number> & fxn_m1 = _nl.getVector(fxn_m1_tagid);
  NumericVector<Number> & xn_m1 = _nl.getVector(xn_m1_tagid);

  for (const auto & dof : transformed_dofs)
  {
    // Avoid 0 denominator issue
    Real new_value = solution(dof);
    if (!MooseUtils::absoluteFuzzyEqual(solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof), 0))
      new_value = xn_m1(dof) - (fxn_m1(dof) - xn_m1(dof)) * (fxn_m1(dof) - xn_m1(dof)) /
                                   (solution(dof) + xn_m1(dof) - 2 * fxn_m1(dof));

    // Relax update
    new_value = relaxation_factor * new_value + (1 - relaxation_factor) * fxn_m1(dof);

    solution.set(dof, new_value);
  }
  solution.close();
  _nl.update();
}

void
SteffensenSolve::printFixedPointConvergenceHistory()
{
  _console << "\n 0 Steffensen initialization |R| = "
           << Console::outputNorm(std::numeric_limits<Real>::max(), _fixed_point_initial_norm)
           << '\n';

  for (unsigned int i = 0; i <= _fixed_point_it; ++i)
  {
    Real max_norm =
        std::max(_fixed_point_timestep_begin_norm[i], _fixed_point_timestep_end_norm[i]);
    std::stringstream steffensen_prefix;
    if (i == 0)
      steffensen_prefix << " Steffensen initialization |R| = ";
    else if (i % 2 == 0)
      steffensen_prefix << " Steffensen half-step      |R| = ";
    else
      steffensen_prefix << " Steffensen step           |R| = ";

    _console << std::setw(2) << i + 1 << steffensen_prefix.str()
             << Console::outputNorm(_fixed_point_initial_norm, max_norm) << '\n';
  }
}
