//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearAssemblySegregatedSolve.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"
#include "LinearSystem.h"

using namespace libMesh;

InputParameters
LinearAssemblySegregatedSolve::validParams()
{
  InputParameters params = SIMPLESolveBase::validParams();
  return params;
}

LinearAssemblySegregatedSolve::LinearAssemblySegregatedSolve(Executioner & ex)
  : SIMPLESolveBase(ex),
    _pressure_sys_number(_problem.linearSysNum(getParam<SolverSystemName>("pressure_system"))),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number)),
    _energy_sys_number(_has_energy_system
                           ? _problem.linearSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _energy_system(_has_energy_system ? &_problem.getLinearSystem(_energy_sys_number) : nullptr)
{
  // We fetch the systems and their numbers for the momentum equations.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.linearSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(&_problem.getLinearSystem(_momentum_system_numbers[system_i]));
    _systems_to_solve.push_back(_momentum_systems.back());
  }

  _systems_to_solve.push_back(&_pressure_system);

  if (_has_energy_system)
    _systems_to_solve.push_back(_energy_system);

  // and for the passive scalar equations
  if (_has_passive_scalar_systems)
    for (auto system_i : index_range(_passive_scalar_system_names))
    {
      _passive_scalar_system_numbers.push_back(
          _problem.linearSysNum(_passive_scalar_system_names[system_i]));
      _passive_scalar_systems.push_back(
          &_problem.getLinearSystem(_passive_scalar_system_numbers[system_i]));
      _systems_to_solve.push_back(_passive_scalar_systems.back());
    }
}

void
LinearAssemblySegregatedSolve::linkRhieChowUserObject()
{
  _rc_uo =
      const_cast<RhieChowMassFlux *>(&getUserObject<RhieChowMassFlux>("rhie_chow_user_object"));
  _rc_uo->linkMomentumPressureSystems(
      _momentum_systems, _pressure_system, _momentum_system_numbers);

  // Initialize the face velocities in the RC object
  if (!_app.isRecovering())
    _rc_uo->initFaceMassFlux();
  _rc_uo->initCouplingField();
}

std::vector<std::pair<unsigned int, Real>>
LinearAssemblySegregatedSolve::solveMomentumPredictor()
{
  // Temporary storage for the (flux-normalized) residuals from
  // different momentum components
  std::vector<std::pair<unsigned int, Real>> its_normalized_residuals;

  LinearImplicitSystem & momentum_system_0 =
      libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[0]->system());

  PetscLinearSolver<Real> & momentum_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system_0.get_linear_solver());

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

    // We assemble the matrix and the right hand side
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs);

    // Still need to relax the right hand side with the same vector
    NS::FV::relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
    NS::FV::relaxRightHandSide(rhs, solution, *diff_diagonal);

    // The normalization factor depends on the right hand side so we need to recompute it for this
    // component
    Real norm_factor = NS::FV::computeNormalizationFactor(solution, mmat, rhs);

    // Very important, for deciding the convergence, we need the unpreconditioned
    // norms in the linear solve
    LibmeshPetscCall(KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED));
    // Solve this component. We don't update the ghosted solution yet, that will come at the end
    // of the corrector step. Also setting the linear tolerances and maximum iteration counts.
    _momentum_linear_control.real_valued_data["abs_tol"] = _momentum_l_abs_tol * norm_factor;
    momentum_solver.set_solver_configuration(_momentum_linear_control);

    // We solve the equation
    auto its_resid_pair = momentum_solver.solve(mmat, mmat, solution, rhs);
    momentum_system.update();

    // We will reuse the preconditioner for every momentum system
    if (system_i == 0)
      momentum_solver.reuse_preconditioner(true);

    // Save the normalized residual
    its_normalized_residuals.push_back(
        std::make_pair(its_resid_pair.first, momentum_solver.get_initial_residual() / norm_factor));

    if (_print_fields)
    {
      _console << " solution after solve " << std::endl;
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

    // Printing residuals
    _console << " Momentum equation:"
             << (_momentum_systems.size() > 1
                     ? std::string(" Component ") + std::to_string(system_i + 1) + std::string(" ")
                     : std::string(" "))
             << COLOR_GREEN << its_normalized_residuals[system_i].second << COLOR_DEFAULT
             << " Linear its: " << its_normalized_residuals[system_i].first << std::endl;
  }

  for (const auto system_i : index_range(_momentum_systems))
  {
    LinearImplicitSystem & momentum_system =
        libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());
    _momentum_systems[system_i]->setSolution(*(momentum_system.current_local_solution));
    _momentum_systems[system_i]->copyPreviousNonlinearSolutions();
  }

  // We reset this to ensure the preconditioner is recomputed new time we go to the momentum
  // predictor
  momentum_solver.reuse_preconditioner(false);

  return its_normalized_residuals;
}

std::pair<unsigned int, Real>
LinearAssemblySegregatedSolve::solvePressureCorrector()
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
  Real norm_factor = NS::FV::computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LibmeshPetscCall(KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  _pressure_linear_control.real_valued_data["abs_tol"] = _pressure_l_abs_tol * norm_factor;
  pressure_solver.set_solver_configuration(_pressure_linear_control);

  if (_pin_pressure)
    NS::FV::constrainSystem(mmat, rhs, _pressure_pin_value, _pressure_pin_dof);
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

  const auto residuals =
      std::make_pair(its_res_pair.first, pressure_solver.get_initial_residual() / norm_factor);

  _console << " Pressure equation: " << COLOR_GREEN << residuals.second << COLOR_DEFAULT
           << " Linear its: " << residuals.first << std::endl;

  return residuals;
}

std::pair<unsigned int, Real>
LinearAssemblySegregatedSolve::correctVelocity(const bool subtract_updated_pressure,
                                               const bool recompute_face_mass_flux,
                                               const SolverParams & solver_params)
{
  // Compute the coupling fields between the momentum and pressure equations.
  // The first argument makes sure the pressure gradient is staged at the first
  // iteration
  _rc_uo->computeHbyA(subtract_updated_pressure, _print_fields);

  // We set the preconditioner/controllable parameters for the pressure equations through
  // petsc options. Linear tolerances will be overridden within the solver.
  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  // Solve the pressure corrector
  const auto residuals = solvePressureCorrector();

  // Compute the face velocity which is used in the advection terms. In certain
  // segregated solver algorithms (like PISO) this is only done on the last iteration.
  if (recompute_face_mass_flux)
    _rc_uo->computeFaceMassFlux();

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  // Relax the pressure update for the next momentum predictor
  NS::FV::relaxSolutionUpdate(
      pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

  // Overwrite old solution
  pressure_old_solution = pressure_current_solution;
  _pressure_system.setSolution(pressure_current_solution);

  // We recompute the updated pressure gradient
  _pressure_system.computeGradients();

  // Reconstruct the cell velocity as well to accelerate convergence
  _rc_uo->computeCellVelocity();

  return residuals;
}

std::pair<unsigned int, Real>
LinearAssemblySegregatedSolve::solveAdvectedSystem(const unsigned int system_num,
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

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = solution.zero_clone();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*li_system.get_linear_solver());

  _problem.computeLinearSystemSys(li_system, mmat, rhs, true);

  // Go and relax the system matrix and the right hand side
  NS::FV::relaxMatrix(mmat, relaxation_factor, *diff_diagonal);
  NS::FV::relaxRightHandSide(rhs, solution, *diff_diagonal);

  if (_print_fields)
  {
    _console << system.name() << " system matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = NS::FV::computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LibmeshPetscCall(KSPSetNormType(linear_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

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

  const auto residuals =
      std::make_pair(its_res_pair.first, linear_solver.get_initial_residual() / norm_factor);

  _console << " Advected system: " << system.name() << " " << COLOR_GREEN << residuals.second
           << COLOR_DEFAULT << " Linear its: " << residuals.first << std::endl;

  return residuals;
}

bool
LinearAssemblySegregatedSolve::solve()
{
  // Do not solve if problem is set not to
  if (!_problem.shouldSolve())
    return true;

  // Dummy solver parameter file which is needed for switching petsc options
  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

  // Initialize the SIMPLE iteration counter
  unsigned int simple_iteration_counter = 0;

  // Assign residuals to general residual vector
  const unsigned int no_systems = _momentum_systems.size() + 1 + _has_energy_system;
  std::vector<std::pair<unsigned int, Real>> ns_residuals(no_systems, std::make_pair(0, 1.0));
  std::vector<Real> ns_abs_tols(_momentum_systems.size(), _momentum_absolute_tolerance);
  ns_abs_tols.push_back(_pressure_absolute_tolerance);
  if (_has_energy_system)
    ns_abs_tols.push_back(_energy_absolute_tolerance);

  bool converged = false;
  // Loop until converged or hit the maximum allowed iteration number
  while (simple_iteration_counter < _num_iterations && !converged)
  {
    simple_iteration_counter++;

    // We set the preconditioner/controllable parameters through petsc options. Linear
    // tolerances will be overridden within the solver. In case of a segregated momentum
    // solver, we assume that every velocity component uses the same preconditioner
    Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

    // Initialize pressure gradients, after this we just reuse the last ones from each
    // iteration
    if (simple_iteration_counter == 1)
      _pressure_system.computeGradients();

    _console << "Iteration " << simple_iteration_counter << " Initial residual norms:" << std::endl;

    // Solve the momentum predictor step
    auto momentum_residual = solveMomentumPredictor();
    for (const auto system_i : index_range(momentum_residual))
      ns_residuals[system_i] = momentum_residual[system_i];

    // Now we correct the velocity, this function depends on the method, it differs for
    // SIMPLE/PIMPLE, this returns the pressure errors
    ns_residuals[momentum_residual.size()] = correctVelocity(true, true, solver_params);

    // If we have an energy equation, solve it here.We assume the material properties in the
    // Navier-Stokes equations depend on temperature, therefore we can not solve for temperature
    // outside of the velocity-pressure loop
    if (_has_energy_system)
    {
      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
      ns_residuals[momentum_residual.size() + 1] = solveAdvectedSystem(_energy_sys_number,
                                                                       *_energy_system,
                                                                       _energy_equation_relaxation,
                                                                       _energy_linear_control,
                                                                       _energy_l_abs_tol);
    }
    _problem.execute(EXEC_NONLINEAR);

    converged = NS::FV::converged(ns_residuals, ns_abs_tols);
  }

  // If we have passive scalar equations, solve them here. We assume the material properties in the
  // Navier-Stokes equations do not depend on passive scalars, as they are passive, therefore we
  // solve outside of the velocity-pressure loop
  if (_has_passive_scalar_systems && converged)
  {
    // The reason why we need more than one iteration is due to the matrix relaxation
    // which can be used to stabilize the equations
    bool passive_scalar_converged = false;
    unsigned int ps_iteration_counter = 0;

    _console << "Passive scalar iteration " << ps_iteration_counter
             << " Initial residual norms:" << std::endl;

    while (ps_iteration_counter < _num_iterations && !passive_scalar_converged)
    {
      ps_iteration_counter++;
      std::vector<std::pair<unsigned int, Real>> scalar_residuals(
          _passive_scalar_system_names.size(), std::make_pair(0, 1.0));
      std::vector<Real> scalar_abs_tols;
      for (const auto scalar_tol : _passive_scalar_absolute_tolerance)
        scalar_abs_tols.push_back(scalar_tol);

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_passive_scalar_petsc_options, solver_params);
      for (const auto i : index_range(_passive_scalar_system_names))
        scalar_residuals[i] = solveAdvectedSystem(_passive_scalar_system_numbers[i],
                                                  *_passive_scalar_systems[i],
                                                  _passive_scalar_equation_relaxation[i],
                                                  _passive_scalar_linear_control,
                                                  _passive_scalar_l_abs_tol);

      passive_scalar_converged = NS::FV::converged(scalar_residuals, scalar_abs_tols);
    }

    // Both flow and scalars must converge
    converged = passive_scalar_converged && converged;
  }

  converged = _continue_on_max_its ? true : converged;

  return converged;
}
