//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIMPLESolve.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"
#include "LinearSystem.h"

#include <limits>

using namespace libMesh;

InputParameters
PIMPLESolve::validParams()
{
  InputParameters params = LinearAssemblySegregatedSolve::validParams();
  params.addParam<unsigned int>(
      "num_piso_iterations",
      0,
      "The maximum number of additional PISO pressure-correction stages without recomputing the "
      "momentum matrix.");
  params.addParam<unsigned int>(
      "num_pressure_nonorthogonal_correctors",
      0,
      "Number of additional non-final pressure equation solves inside each pressure-corrector "
      "stage. A value of 0 means one final pressure solve, 1 means one non-final solve followed "
      "by one final solve, etc.");

  return params;
}

PIMPLESolve::PIMPLESolve(Executioner & ex)
  : LinearAssemblySegregatedSolve(ex),
    _num_piso_iterations(getParam<unsigned int>("num_piso_iterations")),
    _num_pressure_nonorthogonal_correctors(
        getParam<unsigned int>("num_pressure_nonorthogonal_correctors"))
{
}

std::pair<unsigned int, Real>
PIMPLESolve::correctVelocity(const bool subtract_updated_pressure,
                             const bool recompute_face_mass_flux,
                             const SolverParams & solver_params)
{
  storePressurePreviousOuterIterationState();

  std::pair<unsigned int, Real> residual;
  unsigned int piso_iteration_counter = 0;
  while (piso_iteration_counter <= _num_piso_iterations)
  {
    _current_piso_iteration = piso_iteration_counter + 1;
    preparePressureCorrectorState(piso_iteration_counter == 0 ? subtract_updated_pressure : false);
    residual = applyPressureCorrectionStage(recompute_face_mass_flux, true, solver_params);
    piso_iteration_counter++;
  }

  _current_piso_iteration = 0;

  return residual;
}

void
PIMPLESolve::preparePressureCorrectorState(const bool subtract_updated_pressure)
{
  _rc_uo->computeHbyA(subtract_updated_pressure, _print_fields);
}

std::pair<unsigned int, Real>
PIMPLESolve::applyPressureCorrectionStage(const bool recompute_face_mass_flux,
                                          const bool publish_pressure_corrected_state,
                                          const SolverParams & solver_params)
{
  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  std::pair<unsigned int, Real> residuals{0, std::numeric_limits<Real>::quiet_NaN()};
  unsigned int total_linear_iterations = 0;

  for (const auto nonorthogonal_iteration : make_range(_num_pressure_nonorthogonal_correctors + 1))
  {
    const bool final_nonorthogonal_iteration =
        nonorthogonal_iteration == _num_pressure_nonorthogonal_correctors;

    residuals = solvePressureCorrector();
    total_linear_iterations += residuals.first;

    postPressureCorrectorSolve(final_nonorthogonal_iteration);

    if (!final_nonorthogonal_iteration)
      continue;

    if (publish_pressure_corrected_state)
      publishPressureCorrectedState(recompute_face_mass_flux);
    else if (recompute_face_mass_flux && _rc_uo)
      _rc_uo->computeFaceMassFlux();
  }

  residuals.first = total_linear_iterations;

  return residuals;
}

void
PIMPLESolve::postPressureCorrectorSolve(const bool /*final_nonorthogonal_iteration*/)
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();

  if (_rc_uo)
    _rc_uo->cachePressureEquationFlux();
}

void
PIMPLESolve::publishPressureCorrectedState(const bool recompute_face_mass_flux)
{
  if (recompute_face_mass_flux && _rc_uo)
    _rc_uo->computeFaceMassFlux();

  relaxPressureFieldForNextPredictor();

  if (_rc_uo)
    _rc_uo->computeCellVelocity();
}

void
PIMPLESolve::storePressurePreviousOuterIterationState()
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  pressure_current_solution.close();

  if (!_pressure_previous_outer_solution)
    _pressure_previous_outer_solution = pressure_current_solution.zero_clone();

  *_pressure_previous_outer_solution = pressure_current_solution;
  _pressure_previous_outer_solution->close();
}

void
PIMPLESolve::relaxPressureFieldForNextPredictor()
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  pressure_current_solution.close();

  if (!_pressure_previous_outer_solution)
    storePressurePreviousOuterIterationState();

  NS::FV::relaxSolutionUpdate(
      pressure_current_solution, *_pressure_previous_outer_solution, _pressure_variable_relaxation);

  if (auto * pressure_old_solution = _pressure_system.solutionPreviousNewton())
  {
    *pressure_old_solution = pressure_current_solution;
    pressure_old_solution->close();
  }

  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();
}

void
PIMPLESolve::advanceSystemOuterIterationHistory(const std::vector<LinearSystem *> & systems) const
{
  for (auto * system : systems)
  {
    unsigned int max_state = 0;
    while (system->hasSolutionState(max_state + 1, Moose::SolutionIterationType::Nonlinear))
      ++max_state;

    for (unsigned int state = max_state; state > 1; --state)
    {
      auto & nonlinear_state =
          system->solutionState(state, Moose::SolutionIterationType::Nonlinear);
      nonlinear_state = system->solutionState(state - 1, Moose::SolutionIterationType::Nonlinear);
      nonlinear_state.close();
    }

    if (max_state >= 1)
    {
      auto & previous_outer_solution =
          system->solutionState(1, Moose::SolutionIterationType::Nonlinear);
      previous_outer_solution = *(system->system().current_local_solution);
      previous_outer_solution.close();
    }
  }
}

void
PIMPLESolve::advancePressureOuterIterationHistory() const
{
  advanceSystemOuterIterationHistory({&_pressure_system});
}
