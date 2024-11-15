//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
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

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = SegregatedSolverBase::validParams();

  params.addClassDescription("Solves the Navier-Stokes equations using the SIMPLE algorithm and "
                             "linear finite volume variables.");

  /*
   * We suppress parameters which are not supported yet
   */
  params.suppressParameter<SolverSystemName>("solid_energy_system");
  params.suppressParameter<std::vector<SolverSystemName>>("passive_scalar_systems");
  params.suppressParameter<std::vector<SolverSystemName>>("turbulence_systems");
  params.suppressParameter<std::vector<Real>>("passive_scalar_equation_relaxation");
  params.suppressParameter<std::vector<Real>>("turbulence_equation_relaxation");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("solid_energy_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("passive_scalar_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("turbulence_petsc_options_value");
  params.suppressParameter<Real>("solid_energy_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("passive_scalar_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("turbulence_absolute_tolerance");
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

SIMPLE::SIMPLE(const InputParameters & parameters)
  : SegregatedSolverBase(parameters),
    _pressure_sys_number(_problem.linearSysNum(getParam<SolverSystemName>("pressure_system"))),
    _energy_sys_number(_has_energy_system
                           ? _problem.linearSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number)),
    _energy_system(_has_energy_system ? &_problem.getLinearSystem(_energy_sys_number) : nullptr)
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
SIMPLE::init()
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

std::vector<std::pair<unsigned int, Real>>
SIMPLE::solveMomentumPredictor()
{
  // Temporary storage for the (flux-normalized) residuals form
  // different momentum components
  std::vector<std::pair<unsigned int, Real>> its_normalized_residuals;

  LinearImplicitSystem & momentum_system_0 =
      libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[0]->system());

  PetscLinearSolver<Real> & momentum_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system_0.get_linear_solver());

  momentum_solver.reuse_preconditioner(true);

  // Solve the momentum equations.
  // TO DO: These equations are VERY similar. If we can store the differences (things coming from
  // BCs for example) separately, it is enough to construct one matrix.
  for (const auto system_i : index_range(_momentum_systems))
  {
    _problem.setCurrentLinearSystem(_momentum_system_numbers[system_i]);

    // We will need the right hand side and the solution of the next component
    LinearImplicitSystem & momentum_system =
        libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());

    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();

    mmat.zero();
    rhs.zero();

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
    LIBMESH_CHKERR(KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED));
    // Solve this component. We don't update the ghosted solution yet, that will come at the end
    // of the corrector step. Also setting the linear tolerances and maximum iteration counts.
    _momentum_linear_control.real_valued_data["abs_tol"] = _momentum_l_abs_tol * norm_factor;
    momentum_solver.set_solver_configuration(_momentum_linear_control);

    // We solve the equation
    auto its_resid_pair = momentum_solver.solve(mmat, mmat, solution, rhs);
    momentum_system.update();

    // Save the normalized residual
    its_normalized_residuals.push_back(
        std::make_pair(its_resid_pair.first, momentum_solver.get_initial_residual() / norm_factor));

    if (_print_fields)
    {
      _console << " solution before solve " << std::endl;
      solution.print();
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

  return its_normalized_residuals;
}

std::pair<unsigned int, Real>
SIMPLE::solvePressureCorrector()
{
  _problem.setCurrentLinearSystem(_pressure_sys_number);

  // We will need some members from the linear system
  LinearImplicitSystem & pressure_system =
      libMesh::cast_ref<LinearImplicitSystem &>(_pressure_system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(pressure_system.current_local_solution);
  NumericVector<Number> & solution = *(pressure_system.solution);
  SparseMatrix<Number> & mmat = *(pressure_system.matrix);
  NumericVector<Number> & rhs = *(pressure_system.rhs);

  mmat.zero();
  rhs.zero();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & pressure_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  _problem.computeLinearSystemSys(pressure_system, mmat, rhs, false);

  if (_print_fields)
  {
    _console << "Pressure matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LIBMESH_CHKERR(KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  _pressure_linear_control.real_valued_data["abs_tol"] = _pressure_l_abs_tol * norm_factor;
  pressure_solver.set_solver_configuration(_pressure_linear_control);

  if (_pin_pressure)
    constrainSystem(mmat, rhs, _pressure_pin_value, _pressure_pin_dof);
  pressure_system.update();

  auto its_res_pair = pressure_solver.solve(mmat, mmat, solution, rhs);
  pressure_system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve pressure " << std::endl;
    rhs.print();
    _console << " Pressure " << std::endl;
    solution.print();
    _console << "Norm factor " << norm_factor << std::endl;
  }

  _pressure_system.setSolution(current_local_solution);

  return std::make_pair(its_res_pair.first, pressure_solver.get_initial_residual() / norm_factor);
}

std::pair<unsigned int, Real>
SIMPLE::solveAdvectedSystem(const unsigned int system_num,
                            LinearSystem & system,
                            const Real relaxation_factor,
                            SolverConfiguration & solver_config,
                            const Real absolute_tol)
{
  _problem.setCurrentLinearSystem(system_num);

  // We will need some members from the implicit linear system
  LinearImplicitSystem & li_system = libMesh::cast_ref<LinearImplicitSystem &>(system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(li_system.current_local_solution);
  NumericVector<Number> & solution = *(li_system.solution);
  SparseMatrix<Number> & mmat = *(li_system.matrix);
  NumericVector<Number> & rhs = *(li_system.rhs);

  mmat.zero();
  rhs.zero();

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = solution.zero_clone();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*li_system.get_linear_solver());

  _problem.computeLinearSystemSys(li_system, mmat, rhs, true);

  // Go and relax the system matrix and the right hand side
  relaxMatrix(mmat, relaxation_factor, *diff_diagonal);
  relaxRightHandSide(rhs, solution, *diff_diagonal);

  if (_print_fields)
  {
    _console << system.name() << " system matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LIBMESH_CHKERR(KSPSetNormType(linear_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  solver_config.real_valued_data["abs_tol"] = absolute_tol * norm_factor;
  linear_solver.set_solver_configuration(solver_config);

  // Solve the system and update current local solution
  auto its_res_pair = linear_solver.solve(mmat, mmat, solution, rhs);
  li_system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve " << system.name() << std::endl;
    rhs.print();
    _console << system.name() << " solution " << std::endl;
    solution.print();
    _console << " Norm factor " << norm_factor << std::endl;
  }

  system.setSolution(current_local_solution);

  return std::make_pair(its_res_pair.first, linear_solver.get_initial_residual() / norm_factor);
}

void
SIMPLE::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover SIMPLE solves!\nExiting...\n" << std::endl;
    return;
  }

  if (_problem.adaptivity().isOn())
  {
    _console << "\nCannot use SIMPLE solves with mesh adaptivity!\nExiting...\n" << std::endl;
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
    unsigned int no_systems = _momentum_systems.size() + 1 + _has_energy_system;
    std::vector<std::pair<unsigned int, Real>> ns_residuals(no_systems, std::make_pair(0, 1.0));
    std::vector<Real> ns_abs_tols(_momentum_systems.size(), _momentum_absolute_tolerance);
    ns_abs_tols.push_back(_pressure_absolute_tolerance);
    if (_has_energy_system)
      ns_abs_tols.push_back(_energy_absolute_tolerance);

    // Loop until converged or hit the maximum allowed iteration number
    while (iteration_counter < _num_iterations && !converged(ns_residuals, ns_abs_tols))
    {
      iteration_counter++;
      size_t residual_index = 0;

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver. In case of a segregated momentum
      // solver, we assume that every velocity component uses the same preconditioner
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

      if (iteration_counter == 1)
        // Solve the momentum predictor step
        _pressure_system.computeGradients();

      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_residuals[system_i] = momentum_residual[system_i];

      // Compute the coupling fields between the momentum and pressure equations
      _rc_uo->computeHbyA(_print_fields);

      // We set the preconditioner/controllable parameters for the pressure equations through
      // petsc options. Linear tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

      // Solve the pressure corrector
      ns_residuals[momentum_residual.size()] = solvePressureCorrector();

      // Compute the face velocity which is used in the advection terms
      _rc_uo->computeFaceMassFlux();

      auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
      auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

      // Relax the pressure update for the next momentum predictor
      relaxSolutionUpdate(
          pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

      // Overwrite old solution
      pressure_old_solution = pressure_current_solution;
      _pressure_system.setSolution(pressure_current_solution);

      // We clear out the caches so that the gradients can be computed with the relaxed solution
      _pressure_system.computeGradients();

      // Reconstruct the cell velocity as well to accelerate convergence
      _rc_uo->computeCellVelocity();

      // Update residual index
      residual_index = momentum_residual.size();

      // If we have an energy equation, solve it here. We assume the material properties in the
      // Navier-Stokes equations depend on temperature, therefore we can not solve for temperature
      // outside of the velocity-pressure loop
      if (_has_energy_system)
      {
        // We set the preconditioner/controllable parameters through petsc options. Linear
        // tolerances will be overridden within the solver.
        Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
        residual_index += 1;
        ns_residuals[residual_index] = solveAdvectedSystem(_energy_sys_number,
                                                           *_energy_system,
                                                           _energy_equation_relaxation,
                                                           _energy_linear_control,
                                                           _energy_l_abs_tol);
      }
      _problem.execute(EXEC_NONLINEAR);
      // Printing residuals
      _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
      for (auto system_i : index_range(_momentum_systems))
        _console << " Momentum equation:"
                 << (_momentum_systems.size() > 1
                         ? std::string(" Component ") + std::to_string(system_i + 1) +
                               std::string(" ")
                         : std::string(" "))
                 << COLOR_GREEN << ns_residuals[system_i].second << COLOR_DEFAULT
                 << " Linear its: " << ns_residuals[system_i].first << std::endl;
      _console << " Pressure equation: " << COLOR_GREEN
               << ns_residuals[momentum_residual.size()].second << COLOR_DEFAULT
               << " Linear its: " << ns_residuals[momentum_residual.size()].first << std::endl;
      residual_index = momentum_residual.size();

      if (_has_energy_system)
      {
        residual_index += 1;
        _console << " Energy equation: " << COLOR_GREEN << ns_residuals[residual_index].second
                 << COLOR_DEFAULT << " Linear its: " << ns_residuals[residual_index].first
                 << std::endl;
      }
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
