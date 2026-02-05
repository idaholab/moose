//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "Executioner.h"

using namespace libMesh;

InputParameters
LinearAssemblySegregatedSolve::validParams()
{
  InputParameters params = SIMPLESolveBase::validParams();

  params.addParam<std::vector<SolverSystemName>>(
      "active_scalar_systems", {}, "The solver system for each active scalar advection equation.");

  /*
   * Parameters to control the solution of each scalar advection system
   */
  params.addParam<std::vector<Real>>("active_scalar_equation_relaxation",
                                     std::vector<Real>(),
                                     "The relaxation which should be used for the active scalar "
                                     "equations. (=1 for no relaxation, "
                                     "diagonal dominance will still be enforced)");

  params.addParam<MultiMooseEnum>("active_scalar_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the active scalar equation(s)");
  params.addParam<MultiMooseEnum>(
      "active_scalar_petsc_options_iname",
      Moose::PetscSupport::getCommonPetscKeys(),
      "Names of PETSc name/value pairs for the active scalar equation(s)");
  params.addParam<std::vector<std::string>>(
      "active_scalar_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "active scalar equation(s)");
  params.addParam<std::vector<Real>>(
      "active_scalar_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the active scalar equation(s).");
  params.addRangeCheckedParam<Real>("active_scalar_l_tol",
                                    1e-5,
                                    "0.0<=active_scalar_l_tol & active_scalar_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the active scalar equation(s).");
  params.addRangeCheckedParam<Real>("active_scalar_l_abs_tol",
                                    1e-10,
                                    "0.0<active_scalar_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the active scalar equation(s).");
  params.addParam<unsigned int>(
      "active_scalar_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the turbulence equation.");

  params.addParamNamesToGroup(
      "active_scalar_systems active_scalar_equation_relaxation active_scalar_petsc_options "
      "active_scalar_petsc_options_iname "
      "active_scalar_petsc_options_value active_scalar_petsc_options_value "
      "active_scalar_absolute_tolerance "
      "active_scalar_l_tol active_scalar_l_abs_tol active_scalar_l_max_its",
      "Active Scalars Equations");

  /*
   * Flags to optionally skip solving subsets of the thermal-hydraulics system (useful when
   * recovering a converged solution and only advancing scalar transport for example).
   */
  params.addParam<bool>(
      "should_solve_momentum", true, "Whether we should solve the momentum predictor/corrector.");
  params.addParam<bool>(
      "should_solve_pressure", true, "Whether we should solve the pressure corrector.");
  params.addParam<bool>(
      "should_solve_energy", true, "Whether we should solve the fluid energy equation.");
  params.addParam<bool>(
      "should_solve_solid_energy", true, "Whether we should solve the solid energy equation.");
  params.addParam<bool>("should_solve_turbulence",
                        true,
                        "Whether we should solve the turbulence surrogate equations.");
  params.addParam<bool>(
      "should_solve_passive_scalars", true, "Whether we should solve passive scalar equations.");
  params.addParam<bool>(
      "should_solve_active_scalars", true, "Whether we should solve active scalar equations.");
  params.addParam<bool>("should_solve_pm_radiation",
                        true,
                        "Whether we should solve participating media radiation equations.");
  params.addParamNamesToGroup("should_solve_momentum should_solve_pressure should_solve_energy "
                              "should_solve_solid_energy should_solve_turbulence "
                              "should_solve_passive_scalars should_solve_active_scalars",
                              "Solve control");

  /*
   * Parameters to control the conjugate heat transfer
   */
  params += NS::FV::CHTHandler::validParams();

  return params;
}

LinearAssemblySegregatedSolve::LinearAssemblySegregatedSolve(Executioner & ex)
  : SIMPLESolveBase(ex),
    _pressure_sys_number(_problem.linearSysNum(getParam<SolverSystemName>("pressure_system"))),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number)),
    _energy_sys_number(_has_energy_system
                           ? _problem.linearSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _energy_system(_has_energy_system ? &_problem.getLinearSystem(_energy_sys_number) : nullptr),
    _solid_energy_sys_number(
        _has_solid_energy_system
            ? _problem.linearSysNum(getParam<SolverSystemName>("solid_energy_system"))
            : libMesh::invalid_uint),
    _solid_energy_system(
        _has_solid_energy_system ? &_problem.getLinearSystem(_solid_energy_sys_number) : nullptr),
    _should_solve_momentum(getParam<bool>("should_solve_momentum")),
    _should_solve_pressure(getParam<bool>("should_solve_pressure")),
    _should_solve_energy(getParam<bool>("should_solve_energy")),
    _should_solve_solid_energy(getParam<bool>("should_solve_solid_energy")),
    _should_solve_turbulence(getParam<bool>("should_solve_turbulence")),
    _should_solve_passive_scalars(getParam<bool>("should_solve_passive_scalars")),
    _should_solve_active_scalars(getParam<bool>("should_solve_active_scalars")),
    _should_solve_pm_radiation(getParam<bool>("should_solve_pm_radiation")),
    _active_scalar_system_names(getParam<std::vector<SolverSystemName>>("active_scalar_systems")),
    _has_active_scalar_systems(!_active_scalar_system_names.empty()),
    _active_scalar_equation_relaxation(
        getParam<std::vector<Real>>("active_scalar_equation_relaxation")),
    _active_scalar_l_abs_tol(getParam<Real>("active_scalar_l_abs_tol")),
    _active_scalar_absolute_tolerance(
        getParam<std::vector<Real>>("active_scalar_absolute_tolerance")),
    _cht(ex.parameters())
{
  if (!_should_solve_momentum && _should_solve_pressure)
    paramError("should_solve_momentum",
               "Pressure correction requires solving the momentum equations.");
  if (_should_solve_momentum && !_should_solve_pressure)
    paramError("should_solve_pressure",
               "Solving momentum without a pressure corrector is not supported.");
  if (_has_solid_energy_system && !_should_solve_energy && _should_solve_solid_energy)
    paramError("should_solve_solid_energy",
               "Solid energy solve cannot be enabled when the fluid energy solve is disabled.");

  // We fetch the systems and their numbers for the momentum equations only if we solve them
  if (_should_solve_momentum)
    for (auto system_i : index_range(_momentum_system_names))
    {
      _momentum_system_numbers.push_back(_problem.linearSysNum(_momentum_system_names[system_i]));
      _momentum_systems.push_back(&_problem.getLinearSystem(_momentum_system_numbers[system_i]));
      _systems_to_solve.push_back(_momentum_systems.back());
    }

  if (_should_solve_pressure)
    _systems_to_solve.push_back(&_pressure_system);

  if (_has_energy_system && _should_solve_energy)
    _systems_to_solve.push_back(_energy_system);

  if (_has_solid_energy_system && _should_solve_solid_energy)
    _systems_to_solve.push_back(_solid_energy_system);
  // and for the turbulence surrogate equations
  if (_has_turbulence_systems && _should_solve_turbulence)
    for (auto system_i : index_range(_turbulence_system_names))
    {
      _turbulence_system_numbers.push_back(
          _problem.linearSysNum(_turbulence_system_names[system_i]));
      _turbulence_systems.push_back(
          &_problem.getLinearSystem(_turbulence_system_numbers[system_i]));
    }

  // and for the passive scalar equations
  if (_has_passive_scalar_systems && _should_solve_passive_scalars)
    for (auto system_i : index_range(_passive_scalar_system_names))
    {
      _passive_scalar_system_numbers.push_back(
          _problem.linearSysNum(_passive_scalar_system_names[system_i]));
      _passive_scalar_systems.push_back(
          &_problem.getLinearSystem(_passive_scalar_system_numbers[system_i]));
      if (_should_solve_passive_scalars)
        _systems_to_solve.push_back(_passive_scalar_systems.back());
    }

  // and for the participating media radiation equations
  if (_has_pm_radiation_systems && _should_solve_pm_radiation)
    for (auto system_i : index_range(_pm_radiation_system_names))
    {
      _pm_radiation_system_numbers.push_back(
          _problem.linearSysNum(_pm_radiation_system_names[system_i]));
      _pm_radiation_systems.push_back(
          &_problem.getLinearSystem(_pm_radiation_system_numbers[system_i]));
      _systems_to_solve.push_back(_pm_radiation_systems.back());
    }

  // and for the active scalar equations
  if (_has_active_scalar_systems && _should_solve_active_scalars)
    for (auto system_i : index_range(_active_scalar_system_names))
    {
      _active_scalar_system_numbers.push_back(
          _problem.linearSysNum(_active_scalar_system_names[system_i]));
      _active_scalar_systems.push_back(
          &_problem.getLinearSystem(_active_scalar_system_numbers[system_i]));
      _systems_to_solve.push_back(_active_scalar_systems.back());

      const auto & active_scalar_petsc_options =
          getParam<MultiMooseEnum>("active_scalar_petsc_options");
      const auto & active_scalar_petsc_pair_options = getParam<MooseEnumItem, std::string>(
          "active_scalar_petsc_options_iname", "active_scalar_petsc_options_value");
      Moose::PetscSupport::addPetscFlagsToPetscOptions(
          active_scalar_petsc_options, "", *this, _active_scalar_petsc_options);
      Moose::PetscSupport::addPetscPairsToPetscOptions(active_scalar_petsc_pair_options,
                                                       _problem.mesh().dimension(),
                                                       "",
                                                       *this,
                                                       _active_scalar_petsc_options);

      _active_scalar_linear_control.real_valued_data["rel_tol"] =
          getParam<Real>("active_scalar_l_tol");
      _active_scalar_linear_control.real_valued_data["abs_tol"] =
          getParam<Real>("active_scalar_l_abs_tol");
      _active_scalar_linear_control.int_valued_data["max_its"] =
          getParam<unsigned int>("active_scalar_l_max_its");
    }

  if (_active_scalar_equation_relaxation.size() != _active_scalar_system_names.size())
    paramError("active_scalar_equation_relaxation",
               "Should be the same size as the number of systems");

  // We disable the prefix here for the time being, the segregated solvers use a different approach
  // for setting the petsc parameters
  for (auto & system : _systems_to_solve)
    system->system().prefix_with_name(false);

  // Link CHT objects, this will also do some error checking
  // Make a copy for compatibility. These could change in the future
  // Convert _pm_radiation_systems to std::vector<SystemBase *>
  if (_cht.enabled())
  {
    if (!_should_solve_energy)
      paramError("should_solve_energy",
                 "Conjugate heat transfer requires solving the fluid energy equation.");
    if (_has_solid_energy_system && !_should_solve_solid_energy)
      paramError("should_solve_solid_energy",
                 "Conjugate heat transfer requires solving the solid energy equation.");

    std::vector<SystemBase *> pm_radiation_systems_base(_pm_radiation_systems.begin(),
                                                        _pm_radiation_systems.end());

    _cht.linkEnergySystems(_solid_energy_system, _energy_system, pm_radiation_systems_base);
  }
}

void
LinearAssemblySegregatedSolve::linkRhieChowUserObject()
{
  if (!_should_solve_momentum)
    return;

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
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs, /*compute_grads*/ true);

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

void
LinearAssemblySegregatedSolve::initialSetup()
{
  if (_cht.enabled())
  {
    _cht.deduceCHTBoundaryCoupling();
    _cht.setupConjugateHeatTransferContainers();
  }
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
LinearAssemblySegregatedSolve::solveSolidEnergy()
{
  _problem.setCurrentLinearSystem(_solid_energy_sys_number);

  // We will need some members from the linear system
  LinearImplicitSystem & system =
      libMesh::cast_ref<LinearImplicitSystem &>(_solid_energy_system->system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(system.current_local_solution);
  NumericVector<Number> & solution = *(system.solution);
  SparseMatrix<Number> & mmat = *(system.matrix);
  NumericVector<Number> & rhs = *(system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*system.get_linear_solver());

  _problem.computeLinearSystemSys(system, mmat, rhs, false);

  if (_print_fields)
  {
    _console << "Solid energy matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = NS::FV::computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LibmeshPetscCall(KSPSetNormType(solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  _solid_energy_linear_control.real_valued_data["abs_tol"] = _solid_energy_l_abs_tol * norm_factor;
  solver.set_solver_configuration(_solid_energy_linear_control);

  auto its_res_pair = solver.solve(mmat, mmat, solution, rhs);
  system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve solid energy " << std::endl;
    rhs.print();
    _console << " Solid energy " << std::endl;
    solution.print();
    _console << "Norm factor " << norm_factor << std::endl;
  }

  _solid_energy_system->setSolution(current_local_solution);

  const auto residuals =
      std::make_pair(its_res_pair.first, solver.get_initial_residual() / norm_factor);

  _console << " Solid energy equation: " << COLOR_GREEN << residuals.second << COLOR_DEFAULT
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
                                                   const Real absolute_tol,
                                                   const Real field_relaxation,
                                                   const Real min_value_limiter)
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

  // Limiting scalar solution
  if (min_value_limiter != std::numeric_limits<Real>::min())
    NS::FV::limitSolutionUpdate(current_local_solution, min_value_limiter);

  // Relax the field update for the next momentum predictor
  if (field_relaxation != 1.0)
  {
    auto & old_local_solution = *(system.solutionPreviousNewton());
    NS::FV::relaxSolutionUpdate(current_local_solution, old_local_solution, field_relaxation);

    // Update old solution, only needed if relaxing the field
    old_local_solution = current_local_solution;
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

  // We set up the residual storage and the corresponding tolerances.
  ResidualStorage residual_storage = setupResidualStorage();
  auto & ns_residuals = residual_storage.ns_residuals;
  auto & ns_abs_tols = residual_storage.ns_abs_tols;
  const auto & momentum_indices = residual_storage.momentum_indices;
  const auto pressure_index = residual_storage.pressure_index;
  const auto energy_index = residual_storage.energy_index;
  const auto solid_energy_index = residual_storage.solid_energy_index;
  const auto & active_scalar_indices = residual_storage.active_scalar_indices;
  const auto & turbulence_indices = residual_storage.turbulence_indices;
  const auto & pm_radiation_indices = residual_storage.pm_radiation_indices;

  bool converged = residual_storage.converged;

  // Loop until converged or hit the maximum allowed iteration number
  if (_cht.enabled() && _should_solve_energy)
    _cht.initializeCHTCouplingFields();

  while (simple_iteration_counter < _num_iterations && !converged)
  {
    simple_iteration_counter++;

    // We set the preconditioner/controllable parameters through petsc options. Linear
    // tolerances will be overridden within the solver. In case of a segregated momentum
    // solver, we assume that every velocity component uses the same preconditioner
    if (_should_solve_momentum)
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

    // Initialize pressure gradients, after this we just reuse the last ones from each
    // iteration
    if (_should_solve_pressure && simple_iteration_counter == 1)
      _pressure_system.computeGradients();

    _console << "Iteration " << simple_iteration_counter << " Initial residual norms:" << std::endl;

    // Solve the momentum predictor step
    if (_should_solve_momentum)
    {
      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_residuals[momentum_indices[system_i]] = momentum_residual[system_i];
    }

    // Now we correct the velocity, this function depends on the method, it differs for
    // SIMPLE/PIMPLE, this returns the pressure errors
    if (_should_solve_pressure)
      ns_residuals[pressure_index] = correctVelocity(true, true, solver_params);

    // If we have an energy equation, solve it here.We assume the material properties in the
    // Navier-Stokes equations depend on temperature, therefore we can not solve for temperature
    // outside of the velocity-pressure loop
    if (_has_energy_system && _should_solve_energy)
    {
      // If there is no CHT specified this will just do go once through this block
      _cht.resetCHTConvergence();
      while (!_cht.converged())
      {
        if (_cht.enabled())
          _cht.updateCHTBoundaryCouplingFields(NS::CHTSide::FLUID);

        // We set the preconditioner/controllable parameters through petsc options. Linear
        // tolerances will be overridden within the solver.
        Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
        ns_residuals[energy_index] = solveAdvectedSystem(_energy_sys_number,
                                                         *_energy_system,
                                                         _energy_equation_relaxation,
                                                         _energy_linear_control,
                                                         _energy_l_abs_tol);

        if (_has_pm_radiation_systems && _should_solve_pm_radiation)
        {
          // We set the preconditioner/controllable parameters through petsc options. Linear
          // tolerances will be overridden within the solver.
          Moose::PetscSupport::petscSetOptions(_pm_radiation_petsc_options, solver_params);
          for (const auto i : index_range(_pm_radiation_system_names))
          {
            ns_residuals[pm_radiation_indices[i]] =
                solveAdvectedSystem(_pm_radiation_system_numbers[i],
                                    *_pm_radiation_systems[i],
                                    _pm_radiation_equation_relaxation[i],
                                    _pm_radiation_linear_control,
                                    _pm_radiation_l_abs_tol);
          }
        }

        if (_has_solid_energy_system && _should_solve_solid_energy)
        {
          // For now we only update gradients if cht is needed, might change in the future
          if (_cht.enabled())
          {
            _energy_system->computeGradients();
            _cht.updateCHTBoundaryCouplingFields(NS::CHTSide::SOLID);
          }

          // We set the preconditioner/controllable parameters through petsc options. Linear
          // tolerances will be overridden within the solver.
          Moose::PetscSupport::petscSetOptions(_solid_energy_petsc_options, solver_params);
          ns_residuals[solid_energy_index] = solveSolidEnergy();

          // For now we only update gradients if cht is needed, might change in the future
          if (_cht.enabled())
            _solid_energy_system->computeGradients();
        }

        if (_cht.enabled())
        {
          _cht.sumIntegratedFluxes();
          _cht.printIntegratedFluxes();
        }

        _cht.incrementCHTIterators();
      }
      if (_cht.enabled())
        _cht.resetIntegratedFluxes();
    }

    // If we have active scalar equations, solve them here in case they depend on temperature
    // or they affect the fluid properties such that they must be solved concurrently with
    // pressure and velocity
    if (_has_active_scalar_systems && _should_solve_active_scalars)
    {
      _problem.execute(EXEC_NONLINEAR);

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_active_scalar_petsc_options, solver_params);
      for (const auto i : index_range(_active_scalar_system_names))
        ns_residuals[active_scalar_indices[i]] =
            solveAdvectedSystem(_active_scalar_system_numbers[i],
                                *_active_scalar_systems[i],
                                _active_scalar_equation_relaxation[i],
                                _active_scalar_linear_control,
                                _active_scalar_l_abs_tol);
    }

    // If we have turbulence equations, solve them here.
    // The turbulent viscosity depends on the value of the turbulence surrogate variables
    if (_has_turbulence_systems && _should_solve_turbulence)
    {
      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_turbulence_petsc_options, solver_params);
      for (const auto i : index_range(_turbulence_system_names))
      {
        ns_residuals[turbulence_indices[i]] =
            solveAdvectedSystem(_turbulence_system_numbers[i],
                                *_turbulence_systems[i],
                                _turbulence_equation_relaxation[i],
                                _turbulence_linear_control,
                                _turbulence_l_abs_tol,
                                _turbulence_field_relaxation[i],
                                _turbulence_field_min_limit[i]);
      }
    }

    _problem.execute(EXEC_NONLINEAR);

    converged = NS::FV::converged(ns_residuals, ns_abs_tols);
  }

  // If we have passive scalar equations, solve them here. We assume the material properties in
  // the Navier-Stokes equations do not depend on passive scalars, as they are passive, therefore
  // we solve outside of the velocity-pressure loop
  if (_has_passive_scalar_systems && _should_solve_passive_scalars &&
      (converged || _continue_on_max_its))
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

LinearAssemblySegregatedSolve::ResidualStorage
LinearAssemblySegregatedSolve::setupResidualStorage() const
{
  ResidualStorage storage;

  // Residual store: position in this vector defines the ordering used by NS::FV::converged()
  // Each entry holds (linear its, normalized residual) for one system
  if (_should_solve_momentum)
    for ([[maybe_unused]] const auto system_i : index_range(_momentum_systems))
    {
      storage.momentum_indices.push_back(storage.ns_residuals.size());
      storage.ns_residuals.push_back(std::make_pair(0, 1.0));
      storage.ns_abs_tols.push_back(_momentum_absolute_tolerance);
    }

  if (_should_solve_pressure)
  {
    storage.pressure_index = storage.ns_residuals.size();
    storage.ns_residuals.push_back(std::make_pair(0, 1.0));
    storage.ns_abs_tols.push_back(_pressure_absolute_tolerance);
  }

  if (_has_energy_system && _should_solve_energy)
  {
    storage.energy_index = storage.ns_residuals.size();
    storage.ns_residuals.push_back(std::make_pair(0, 1.0));
    storage.ns_abs_tols.push_back(_energy_absolute_tolerance);
  }

  if (_has_solid_energy_system && _should_solve_solid_energy)
  {
    storage.solid_energy_index = storage.ns_residuals.size();
    storage.ns_residuals.push_back(std::make_pair(0, 1.0));
    storage.ns_abs_tols.push_back(_solid_energy_absolute_tolerance);
  }

  if (_has_active_scalar_systems && _should_solve_active_scalars)
    for (const auto i : index_range(_active_scalar_system_names))
    {
      storage.active_scalar_indices.push_back(storage.ns_residuals.size());
      storage.ns_residuals.push_back(std::make_pair(0, 1.0));
      storage.ns_abs_tols.push_back(_active_scalar_absolute_tolerance[i]);
    }

  if (_has_turbulence_systems && _should_solve_turbulence)
    for (const auto i : index_range(_turbulence_system_names))
    {
      storage.turbulence_indices.push_back(storage.ns_residuals.size());
      storage.ns_residuals.push_back(std::make_pair(0, 1.0));
      storage.ns_abs_tols.push_back(_turbulence_absolute_tolerance[i]);
    }

  if (_has_pm_radiation_systems && _should_solve_pm_radiation)
    for (const auto i : index_range(_pm_radiation_system_names))
    {
      storage.pm_radiation_indices.push_back(storage.ns_residuals.size());
      storage.ns_residuals.push_back(std::make_pair(0, 1.0));
      storage.ns_abs_tols.push_back(_pm_radiation_absolute_tolerance[i]);
    }

  storage.converged = storage.ns_residuals.empty();
  return storage;
}
