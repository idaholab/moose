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
#include "LinearSystem.h"
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
  // Temporary storage for the (flux-normalized) residuals form
  // different momentum components
  std::vector<Real> normalized_residuals;

  // Solve the momentum equations.
  // TO DO: These equations are VERY similar. If we can store the differences (things coming from
  // BCs for example) separately, it is enough to construct one matrix.
  for (const auto system_i : index_range(_momentum_systems))
  {
    _problem.setCurrentLinearSystem(_momentum_system_numbers[system_i]);

    // We will need the right hand side and the solution of the next component
    LinearImplicitSystem & momentum_system =
        libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());

    PetscLinearSolver<Real> & momentum_solver =
        libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());

    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();

    // We plug zero in this to get the system matrix and the right hand side of the linear problem
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs);

    // Still need to relax the right hand side with the same vector
    relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
    relaxRightHandSide(rhs, solution, *diff_diagonal);

    // The normalization factor depends on the right hand side so we need to recompute it for this
    // component
    Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

    // Very important, for deciding the convergence, we need the unpreconditioned
    // norms in the linear solve
    KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED);
    // Solve this component. We don't update the ghosted solution yet, that will come at the end
    // of the corrector step. Also setting the linear tolerances and maximum iteration counts.
    _momentum_linear_control.real_valued_data["abs_tol"] = _momentum_l_abs_tol * norm_factor;
    momentum_solver.set_solver_configuration(_momentum_linear_control);

    // We solve the equation
    momentum_solver.solve(mmat, mmat, solution, rhs);
    momentum_system.update();

    // Save the normalized residual
    normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

    if (_print_fields)
    {
      _console << " matrix when we solve " << std::endl;
      mmat.print();
      _console << " rhs when we solve " << std::endl;
      rhs.print();
      _console << " velocity solution component " << system_i << std::endl;
      solution.print();
      _console << "Norm factor " << norm_factor << std::endl;
      _console << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
    }
  }

  for (const auto system_i : index_range(_momentum_systems))
  {
    LinearImplicitSystem & momentum_system =
        libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());
    _momentum_systems[system_i]->setSolution(*(momentum_system.current_local_solution));
    _momentum_systems[system_i]->copySolutionsBackwards();
  }

  return normalized_residuals;
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

  if (_problem.shouldSolve())
  {
    // Dummy solver parameter file which is needed for switching petsc options
    SolverParams solver_params;
    solver_params._type = Moose::SolveType::ST_LINEAR;
    solver_params._line_search = Moose::LineSearchType::LS_NONE;

    // Initialize the quantities which matter in terms of the iteration
    unsigned int iteration_counter = 0;

    // Assign residuals to general residual vector
    unsigned int no_systems = _momentum_systems.size() + 1;
    std::vector<Real> ns_residuals(no_systems, 1.0);
    std::vector<Real> ns_abs_tols(_momentum_systems.size(), _momentum_absolute_tolerance);
    ns_abs_tols.push_back(_pressure_absolute_tolerance);

    // Loop until converged or hit the maximum allowed iteration number
    while (iteration_counter < _num_iterations && !converged(ns_residuals, ns_abs_tols))
    {
      iteration_counter++;

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver. In case of a segregated momentum
      // solver, we assume that every velocity component uses the same preconditioner
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

      // Solve the momentum predictor step
      _pressure_system.computeGradients();
      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_residuals[system_i] = momentum_residual[system_i];

      // Compute the coupling fields between the momentum and pressure equations
      _rc_uo->computeHbyA(_print_fields);

      // Printing residuals
      _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
      for (auto system_i : index_range(_momentum_systems))
        _console << " Momentum equation:"
                 << (_momentum_systems.size() > 1
                         ? std::string(" Component ") + std::to_string(system_i + 1) +
                               std::string(" ")
                         : std::string(" "))
                 << COLOR_GREEN << ns_residuals[system_i] << COLOR_DEFAULT << std::endl;
      _console << " Pressure equation: " << COLOR_GREEN << ns_residuals[momentum_residual.size()]
               << COLOR_DEFAULT << std::endl;
    }
  }

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
