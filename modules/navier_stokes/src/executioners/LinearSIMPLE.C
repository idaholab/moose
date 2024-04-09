//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearSIMPLE.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"
#include "libmesh/enum_point_locator_type.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

registerMooseObject("NavierStokesApp", LinearSIMPLE);

InputParameters
LinearSIMPLE::validParams()
{
  InputParameters params = SegregatedSolverBase::validParams();

  params.addClassDescription("Solves the Navier-Stokes equations using the SIMPLE algorithm and "
                             "linear finite volume variables.");

  /*
   * We suppress parameters which are not supported yet
   */
  params.suppressParameter<SolverSystemName>("energy_system");
  params.suppressParameter<SolverSystemName>("solid_energy_system");
  params.suppressParameter<std::vector<SolverSystemName>>("passive_scalar_systems");
  params.suppressParameter<std::vector<SolverSystemName>>("turbulence_systems");
  params.suppressParameter<Real>("energy_equation_relaxation");
  params.suppressParameter<std::vector<Real>>("passive_scalar_equation_relaxation");
  params.suppressParameter<std::vector<Real>>("turbulence_equation_relaxation");
  params.suppressParameter<MultiMooseEnum>("energy_petsc_options");
  params.suppressParameter<MultiMooseEnum>("energy_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("energy_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("solid_energy_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("passive_scalar_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("turbulence_petsc_options_value");
  params.suppressParameter<Real>("energy_absolute_tolerance");
  params.suppressParameter<Real>("solid_energy_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("passive_scalar_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("turbulence_absolute_tolerance");
  params.suppressParameter<Real>("energy_l_tol");
  params.suppressParameter<Real>("energy_l_abs_tol");
  params.suppressParameter<unsigned int>("energy_l_max_its");
  params.suppressParameter<Real>("solid_energy_l_tol");
  params.suppressParameter<Real>("solid_energy_l_abs_tol");
  params.suppressParameter<unsigned int>("solid_energy_l_max_its");
  params.suppressParameter<Real>("passive_scalar_l_tol");
  params.suppressParameter<Real>("passive_scalar_l_abs_tol");
  params.suppressParameter<unsigned int>("passive_scalar_l_max_its");
  params.suppressParameter<Real>("turbulence_l_tol");
  params.suppressParameter<Real>("turbulence_l_abs_tol");
  params.suppressParameter<unsigned int>("turbulence_l_max_its");

  return params;
}

LinearSIMPLE::LinearSIMPLE(const InputParameters & parameters)
  : SegregatedSolverBase(parameters),
    _pressure_sys_number(_problem.linearSysNum(getParam<SolverSystemName>("pressure_system"))),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number))
{
  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.linearSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(&_problem.getLinearSystem(_momentum_system_numbers[system_i]));
  }
}

void
LinearSIMPLE::init()
{
  SegregatedSolverBase::init();

  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo =
      const_cast<RhieChowMassFlux *>(&getUserObject<RhieChowMassFlux>("rhie_chow_user_object"));
  _rc_uo->linkMomentumPressureSystems(
      _momentum_systems, _pressure_system, _momentum_system_numbers);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceMassFlux();
}

std::vector<Real>
LinearSIMPLE::solveMomentumPredictor()
{
  return {0.0};
}

Real
LinearSIMPLE::solvePressureCorrector()
{
  return 0.0;
}

void
LinearSIMPLE::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover LinearSIMPLE solves!\nExiting...\n" << std::endl;
    return;
  }

  if (_problem.adaptivity().isOn())
  {
    _console << "\nCannot use LinearSIMPLE solves with mesh adaptivity!\nExiting...\n" << std::endl;
    return;
  }

  ExecFlagEnum disabled_flags;
  disabled_flags.addAvailableFlags(EXEC_TIMESTEP_BEGIN,
                                   EXEC_TIMESTEP_END,
                                   EXEC_INITIAL,
                                   EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                   EXEC_MULTIAPP_FIXED_POINT_END,
                                   EXEC_LINEAR,
                                   EXEC_NONLINEAR);

  if (hasMultiAppError(disabled_flags))
    return;
  if (hasTransferError(disabled_flags))
    return;

  _problem.timestepSetup();

  _time_step = 0;
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  _time_step = 1;

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  // Dummy solver parameter file which is needed for switching petsc options
  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

  _time = _time_step;
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::TO_MULTIAPP);
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::BETWEEN_MULTIAPP);
    _problem.execMultiApps(EXEC_FINAL);
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::FROM_MULTIAPP);
    _problem.finalizeMultiApps();
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}
