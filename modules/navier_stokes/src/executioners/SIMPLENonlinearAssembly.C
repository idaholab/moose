//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLENonlinearAssembly.h"
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

registerMooseObject("NavierStokesApp", SIMPLENonlinearAssembly);

InputParameters
SIMPLENonlinearAssembly::validParams()
{
  InputParameters params = SegregatedSolverBase::validParams();

  params.addClassDescription("Solves the Navier-Stokes equations using the "
                             "SIMPLENonlinearAssembly algorithm.");

  params.addParam<TagName>("pressure_gradient_tag",
                           "pressure_momentum_kernels",
                           "The name of the tags associated with the kernels in the momentum "
                           "equations which are not related to the pressure gradient.");
  return params;
}

SIMPLENonlinearAssembly::SIMPLENonlinearAssembly(const InputParameters & parameters)
  : SegregatedSolverBase(parameters),
    _pressure_sys_number(_problem.nlSysNum(getParam<SolverSystemName>("pressure_system"))),
    _energy_sys_number(_has_energy_system
                           ? _problem.nlSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _solid_energy_sys_number(
        _has_solid_energy_system
            ? _problem.nlSysNum(getParam<SolverSystemName>("solid_energy_system"))
            : libMesh::invalid_uint),
    _pressure_system(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _energy_system(_has_energy_system ? &_problem.getNonlinearSystemBase(_energy_sys_number)
                                      : nullptr),
    _solid_energy_system(_has_solid_energy_system
                             ? &_problem.getNonlinearSystemBase(_solid_energy_sys_number)
                             : nullptr),
    _pressure_tag_name(getParam<TagName>("pressure_gradient_tag")),
    _pressure_tag_id(_problem.addVectorTag(_pressure_tag_name))
{
  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.nlSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(
        &_problem.getNonlinearSystemBase(_momentum_system_numbers[system_i]));
    _momentum_systems[system_i]->addVector(_pressure_tag_id, false, ParallelType::PARALLEL);
  }

  if (_has_passive_scalar_systems)
    for (auto system_i : index_range(_passive_scalar_system_names))
    {
      _passive_scalar_system_numbers.push_back(
          _problem.nlSysNum(_passive_scalar_system_names[system_i]));
      _passive_scalar_systems.push_back(
          &_problem.getNonlinearSystemBase(_passive_scalar_system_numbers[system_i]));
    }

  if (_has_turbulence_systems)
    for (auto system_i : index_range(_turbulence_system_names))
    {
      _turbulence_system_numbers.push_back(_problem.nlSysNum(_turbulence_system_names[system_i]));
      _turbulence_systems.push_back(
          &_problem.getNonlinearSystemBase(_turbulence_system_numbers[system_i]));
    }
}

void
SIMPLENonlinearAssembly::init()
{
  SegregatedSolverBase::init();

  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  for (const auto system : _momentum_systems)
    checkIntegrity(*system);

  checkIntegrity(_pressure_system);

  if (_has_energy_system)
  {
    checkIntegrity(*_energy_system);
    if (_has_solid_energy_system)
      checkIntegrity(*_solid_energy_system);
  }

  if (_has_passive_scalar_systems)
    for (const auto system : _passive_scalar_systems)
      checkIntegrity(*system);

  if (_has_turbulence_systems)
    for (const auto system : _turbulence_systems)
      checkIntegrity(*system);

  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_system_numbers, _pressure_tag_id);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceVelocities();
}

std::vector<std::pair<unsigned int, Real>>
SIMPLENonlinearAssembly::solveMomentumPredictor()
{
  // Temporary storage for the (flux-normalized) residuals form
  // different momentum components
  std::vector<std::pair<unsigned int, Real>> its_normalized_residuals;

  // We can create this here with the assumption that every momentum component has the same number
  // of dofs
  auto zero_solution = _momentum_systems[0]->system().current_local_solution->zero_clone();

  // Solve the momentum equations.
  // TO DO: These equations are VERY similar. If we can store the differences (things coming from
  // BCs for example) separately, it is enough to construct one matrix.
  for (const auto system_i : index_range(_momentum_systems))
  {
    _problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    // We will need the right hand side and the solution of the next component
    NonlinearImplicitSystem & momentum_system =
        libMesh::cast_ref<NonlinearImplicitSystem &>(_momentum_systems[system_i]->system());

    PetscLinearSolver<Real> & momentum_solver =
        libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());

    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();

    // We plug zero in this to get the system matrix and the right hand side of the linear problem
    _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
    // Sadly, this returns -b so we multiply with -1
    rhs.scale(-1.0);

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
    NonlinearImplicitSystem & momentum_system =
        libMesh::cast_ref<NonlinearImplicitSystem &>(_momentum_systems[system_i]->system());
    _momentum_systems[system_i]->setSolution(*(momentum_system.current_local_solution));
    _momentum_systems[system_i]->copySolutionsBackwards();
  }

  return its_normalized_residuals;
}

std::pair<unsigned int, Real>
SIMPLENonlinearAssembly::solvePressureCorrector()
{
  _problem.setCurrentNonlinearSystem(_pressure_sys_number);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & pressure_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(_pressure_system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(pressure_system.current_local_solution);
  NumericVector<Number> & solution = *(pressure_system.solution);
  SparseMatrix<Number> & mmat = *(pressure_system.matrix);
  NumericVector<Number> & rhs = *(pressure_system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & pressure_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the right hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
  rhs.scale(-1.0);

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
SIMPLENonlinearAssembly::solveAdvectedSystem(const unsigned int system_num,
                                             NonlinearSystemBase & system,
                                             const Real relaxation_factor,
                                             SolverConfiguration & solver_config,
                                             const Real absolute_tol)
{
  _problem.setCurrentNonlinearSystem(system_num);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & ni_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(ni_system.current_local_solution);
  NumericVector<Number> & solution = *(ni_system.solution);
  SparseMatrix<Number> & mmat = *(ni_system.matrix);
  NumericVector<Number> & rhs = *(ni_system.rhs);

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = solution.zero_clone();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*ni_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the right hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
  rhs.scale(-1.0);

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
  ni_system.update();

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

std::pair<unsigned int, Real>
SIMPLENonlinearAssembly::solveSolidEnergySystem()
{
  _problem.setCurrentNonlinearSystem(_solid_energy_sys_number);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & se_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(_solid_energy_system->system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(se_system.current_local_solution);
  NumericVector<Number> & solution = *(se_system.solution);
  SparseMatrix<Number> & mat = *(se_system.matrix);
  NumericVector<Number> & rhs = *(se_system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & se_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*se_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the righ hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mat);
  rhs.scale(-1.0);

  if (_print_fields)
  {
    _console << "Solid energy matrix" << std::endl;
    mat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LIBMESH_CHKERR(KSPSetNormType(se_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  _solid_energy_linear_control.real_valued_data["abs_tol"] = _solid_energy_l_abs_tol * norm_factor;
  se_solver.set_solver_configuration(_solid_energy_linear_control);

  auto its_res_pair = se_solver.solve(mat, mat, solution, rhs);
  se_system.update();

  if (_print_fields)
  {
    _console << " Solid energy rhs " << std::endl;
    rhs.print();
    _console << " Solid temperature " << std::endl;
    solution.print();
    _console << "Norm factor " << norm_factor << std::endl;
  }

  _solid_energy_system->setSolution(current_local_solution);

  return std::make_pair(its_res_pair.first, se_solver.get_initial_residual() / norm_factor);
}

void
SIMPLENonlinearAssembly::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover SIMPLENonlinearAssembly solves!\nExiting...\n" << std::endl;
    return;
  }

  if (_problem.adaptivity().isOn())
  {
    _console << "\nCannot use SIMPLENonlinearAssembly solves with mesh "
                "adaptivity!\nExiting...\n"
             << std::endl;
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

  if (_problem.shouldSolve())
  {

    // Initialize the quantities which matter in terms of the iteration
    unsigned int iteration_counter = 0;

    // Assign residuals to general residual vector
    unsigned int no_systems =
        _momentum_systems.size() + 1 + _has_energy_system + _has_solid_energy_system;
    if (_has_turbulence_systems)
      no_systems += _turbulence_systems.size();
    std::vector<std::pair<unsigned int, Real>> ns_its_residuals(no_systems, std::make_pair(0, 1.0));
    std::vector<Real> ns_abs_tols(_momentum_systems.size(), _momentum_absolute_tolerance);
    ns_abs_tols.push_back(_pressure_absolute_tolerance);
    if (_has_energy_system)
    {
      ns_abs_tols.push_back(_energy_absolute_tolerance);
      if (_has_solid_energy_system)
        ns_abs_tols.push_back(_solid_energy_absolute_tolerance);
    }
    if (_has_turbulence_systems)
      for (auto system_i : index_range(_turbulence_absolute_tolerance))
        ns_abs_tols.push_back(_turbulence_absolute_tolerance[system_i]);

    // Loop until converged or hit the maximum allowed iteration number
    while (iteration_counter < _num_iterations && !converged(ns_its_residuals, ns_abs_tols))
    {
      // Resdiual index
      size_t residual_index = 0;

      // Execute all objects tagged as nonlinear
      // This will execute everything in the problem at nonlinear, including the aux kernels.
      // This way we compute the aux kernels before the momentum equations are solved.
      _problem.execute(EXEC_NONLINEAR);

      // We clear the caches in the momentum and pressure variables
      for (auto system_i : index_range(_momentum_systems))
        _momentum_systems[system_i]->residualSetup();
      _pressure_system.residualSetup();

      // If we solve for energy, we clear the caches there too
      if (_has_energy_system)
      {
        _energy_system->residualSetup();
        if (_has_solid_energy_system)
          _solid_energy_system->residualSetup();
      }

      // If we solve for turbulence, we clear the caches there too
      if (_has_turbulence_systems)
        for (auto system_i : index_range(_turbulence_systems))
          _turbulence_systems[system_i]->residualSetup();

      iteration_counter++;

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver. In case of a segregated momentum
      // solver, we assume that every velocity component uses the same preconditioner
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

      // Solve the momentum predictor step
      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_its_residuals[system_i] = momentum_residual[system_i];

      // Compute the coupling fields between the momentum and pressure equations
      _rc_uo->computeHbyA(_print_fields);

      // We set the preconditioner/controllable parameters for the pressure equations through
      // petsc options. Linear tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

      // Solve the pressure corrector
      ns_its_residuals[momentum_residual.size()] = solvePressureCorrector();
      // We need this to make sure we evaluate cell gradients for the nonorthogonal correction in
      // the face velocity update
      _pressure_system.residualSetup();

      // Compute the face velocity which is used in the advection terms
      _rc_uo->computeFaceVelocity();

      auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
      auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());
      // Relax the pressure update for the next momentum predictor
      relaxSolutionUpdate(
          pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

      // Overwrite old solution
      pressure_old_solution = pressure_current_solution;
      _pressure_system.setSolution(pressure_current_solution);

      // We clear out the caches so that the gradients can be computed with the relaxed solution
      _pressure_system.residualSetup();

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
        ns_its_residuals[residual_index] = solveAdvectedSystem(_energy_sys_number,
                                                               *_energy_system,
                                                               _energy_equation_relaxation,
                                                               _energy_linear_control,
                                                               _energy_l_abs_tol);

        if (_has_solid_energy_system)
        {
          // We set the preconditioner/controllable parameters through petsc options. Linear
          // tolerances will be overridden within the solver.
          Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
          residual_index += 1;
          ns_its_residuals[residual_index] = solveSolidEnergySystem();
        }
      }

      // If we have an turbulence equations, we solve it here. We solve it inside the
      // momentum-pressure loop because it affects the turbulent viscosity
      if (_has_turbulence_systems)
      {
        Moose::PetscSupport::petscSetOptions(_turbulence_petsc_options, solver_params);

        for (auto system_i : index_range(_turbulence_systems))
        {
          residual_index += 1;
          ns_its_residuals[residual_index] =
              solveAdvectedSystem(_turbulence_system_numbers[system_i],
                                  *_turbulence_systems[system_i],
                                  _turbulence_equation_relaxation[system_i],
                                  _turbulence_linear_control,
                                  _turbulence_l_abs_tol);

          auto & current_solution =
              *(_turbulence_systems[system_i]->system().current_local_solution.get());
          limitSolutionUpdate(current_solution, _turbulence_field_min_limit[system_i]);

          // Relax the turbulence update for the next momentum predictor
          auto & old_solution = *(_turbulence_systems[system_i]->solutionPreviousNewton());

          // Relax the pressure update for the next momentum predictor
          relaxSolutionUpdate(
              current_solution, old_solution, _turbulence_equation_relaxation[system_i]);

          // Overwrite old solution
          old_solution = current_solution;
          _turbulence_systems[system_i]->setSolution(current_solution);

          // We clear out the caches so that the gradients can be computed with the relaxed solution
          _turbulence_systems[system_i]->residualSetup();
        }
      }

      // Printing residuals
      residual_index = 0;
      _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
      for (auto system_i : index_range(_momentum_systems))
        _console << " Momentum equation:"
                 << (_momentum_systems.size() > 1
                         ? std::string(" Component ") + std::to_string(system_i + 1) +
                               std::string(" ")
                         : std::string(" "))
                 << COLOR_GREEN << ns_its_residuals[system_i].second << COLOR_DEFAULT << std::endl;
      _console << " Pressure equation: " << COLOR_GREEN
               << ns_its_residuals[momentum_residual.size()].second << COLOR_DEFAULT << std::endl;
      residual_index = momentum_residual.size();

      if (_has_energy_system)
      {
        residual_index += 1;
        _console << " Energy equation: " << COLOR_GREEN << ns_its_residuals[residual_index].second
                 << COLOR_DEFAULT << std::endl;
        if (_has_solid_energy_system)
        {
          residual_index += 1;
          _console << " Solid energy equation: " << COLOR_GREEN
                   << ns_its_residuals[residual_index].second << COLOR_DEFAULT << std::endl;
        }
      }

      if (_has_turbulence_systems)
      {
        _console << "Turbulence Iteration " << std::endl;
        for (auto system_i : index_range(_turbulence_systems))
        {
          residual_index += 1;
          _console << _turbulence_systems[system_i]->name() << " " << COLOR_GREEN
                   << ns_its_residuals[residual_index].second << COLOR_DEFAULT << std::endl;
        }
      }
    }

    // Now we solve for the passive scalar equations, they should not influence the solution of the
    // system above. The reason why we need more than one iteration is due to the matrix relaxation
    // which can be used to stabilize the equations
    if (_has_passive_scalar_systems)
    {
      _console << " Passive Scalar Iteration " << iteration_counter << std::endl;

      // We set the options used by Petsc (preconditioners etc). We assume that every passive
      // scalar equation uses the same options for now.
      Moose::PetscSupport::petscSetOptions(_passive_scalar_petsc_options, solver_params);

      iteration_counter = 0;
      std::vector<std::pair<unsigned int, Real>> passive_scalar_residuals(
          _passive_scalar_systems.size(), std::make_pair(0, 1.0));
      while (iteration_counter < _num_iterations &&
             !converged(passive_scalar_residuals, _passive_scalar_absolute_tolerance))
      {
        // We clear the caches in the passive scalar variables
        for (auto system_i : index_range(_passive_scalar_systems))
          _passive_scalar_systems[system_i]->residualSetup();

        iteration_counter++;

        // Solve the passive scalar equations
        for (auto system_i : index_range(_passive_scalar_systems))
          passive_scalar_residuals[system_i] =
              solveAdvectedSystem(_passive_scalar_system_numbers[system_i],
                                  *_passive_scalar_systems[system_i],
                                  _passive_scalar_equation_relaxation[system_i],
                                  _passive_scalar_linear_control,
                                  _passive_scalar_l_abs_tol);

        _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
        for (auto system_i : index_range(_passive_scalar_systems))
          _console << _passive_scalar_systems[system_i]->name() << " " << COLOR_GREEN
                   << passive_scalar_residuals[system_i].second << COLOR_DEFAULT << std::endl;
      }
    }
  }

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

void
SIMPLENonlinearAssembly::checkIntegrity(NonlinearSystemBase & system)
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (system.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation in system",
               system.name());
}
