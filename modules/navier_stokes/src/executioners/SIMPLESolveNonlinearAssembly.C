//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SIMPLESolveNonlinearAssembly.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"

using namespace libMesh;

InputParameters
SIMPLESolveNonlinearAssembly::validParams()
{
  InputParameters params = SIMPLESolveBase::validParams();

  params.addParam<TagName>("pressure_gradient_tag",
                           "pressure_momentum_kernels",
                           "The name of the tags associated with the kernels in the momentum "
                           "equations which are not related to the pressure gradient.");

  /*
   * The names of the different systems in the segregated solver
   */
  params.addParam<SolverSystemName>("solid_energy_system",
                                    "The solver system for the solid energy equation.");
  params.addParam<std::vector<SolverSystemName>>(
      "turbulence_systems", {}, "The solver system(s) for the turbulence equation(s).");

  /*
   * Relaxation parameters for the different system
   */
  params.addParam<std::vector<Real>>(
      "turbulence_equation_relaxation",
      std::vector<Real>(),
      "The relaxation which should be used for the turbulence equations "
      "equations. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");

  params.addParam<std::vector<Real>>(
      "turbulence_field_min_limit",
      std::vector<Real>(),
      "The lower limit imposed on turbulent quantities. The recommended value for robustness "
      "is 1e-8.");

  params.addParamNamesToGroup("energy_equation_relaxation "
                              "turbulence_equation_relaxation",
                              "Relaxation");

  /*
   * Petsc options for every equations in the system
   */
  params.addParam<MultiMooseEnum>("solid_energy_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the solid energy equation");
  params.addParam<MultiMooseEnum>("solid_energy_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the solid energy equation");
  params.addParam<std::vector<std::string>>(
      "solid_energy_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "solid energy equation");

  params.addParam<MultiMooseEnum>("turbulence_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the turbulence equation(s)");
  params.addParam<MultiMooseEnum>("turbulence_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the turbulence equation(s)");
  params.addParam<std::vector<std::string>>(
      "turbulence_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "turbulence equation");

  params.addParamNamesToGroup(
      "solid_energy_petsc_options solid_energy_petsc_options_iname "
      "solid_energy_petsc_options_value "
      "turbulence_petsc_options turbulence_petsc_options_iname turbulence_petsc_options_value",
      "PETSc Control");

  /*
   * Iteration tolerances for the different equations
   */
  params.addRangeCheckedParam<Real>(
      "solid_energy_absolute_tolerance",
      1e-5,
      "0.0<solid_energy_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the solid energy equation.");
  params.addParam<std::vector<Real>>(
      "turbulence_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the turbulence equation(s).");

  params.addParamNamesToGroup("solid_energy_absolute_tolerance turbulence_absolute_tolerance",
                              "Iteration Control");
  /*
   * Linear iteration tolerances for the different equations
   */
  params.addRangeCheckedParam<Real>("solid_energy_l_tol",
                                    1e-5,
                                    "0.0<=solid_energy_l_tol & solid_energy_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the solid energy equation.");
  params.addRangeCheckedParam<Real>("solid_energy_l_abs_tol",
                                    1e-10,
                                    "0.0<solid_energy_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the solid energy equation.");
  params.addRangeCheckedParam<unsigned int>(
      "solid_energy_l_max_its",
      10000,
      "0<solid_energy_l_max_its",
      "The maximum allowed iterations in the linear solver of the solid energy equation.");
  params.addRangeCheckedParam<Real>("turbulence_l_tol",
                                    1e-5,
                                    "0.0<=turbulence_l_tol & turbulence_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the turbulence equation(s).");
  params.addRangeCheckedParam<Real>("turbulence_l_abs_tol",
                                    1e-10,
                                    "0.0<turbulence_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the turbulence equation(s).");
  params.addParam<unsigned int>(
      "turbulence_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the turbulence equation(s).");

  params.addParamNamesToGroup(
      "solid_energy_l_tol solid_energy_l_abs_tol solid_energy_l_max_its turbulence_l_tol "
      "turbulence_l_abs_tol turbulence_l_max_its",
      "Linear Iteration Control");

  return params;
}

SIMPLESolveNonlinearAssembly::SIMPLESolveNonlinearAssembly(Executioner & ex)
  : SIMPLESolveBase(ex),
    _pressure_sys_number(_problem.nlSysNum(getParam<SolverSystemName>("pressure_system"))),
    _pressure_system(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _has_solid_energy_system(_has_energy_system && isParamValid("solid_energy_system")),
    _has_turbulence_systems(!getParam<std::vector<SolverSystemName>>("turbulence_systems").empty()),
    _energy_sys_number(_has_energy_system
                           ? _problem.nlSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _energy_system(_has_energy_system ? &_problem.getNonlinearSystemBase(_energy_sys_number)
                                      : nullptr),
    _solid_energy_sys_number(
        _has_solid_energy_system
            ? _problem.nlSysNum(getParam<SolverSystemName>("solid_energy_system"))
            : libMesh::invalid_uint),
    _solid_energy_system(_has_solid_energy_system
                             ? &_problem.getNonlinearSystemBase(_solid_energy_sys_number)
                             : nullptr),
    _solid_energy_l_abs_tol(getParam<Real>("solid_energy_l_abs_tol")),
    _turbulence_system_names(getParam<std::vector<SolverSystemName>>("turbulence_systems")),
    _turbulence_equation_relaxation(getParam<std::vector<Real>>("turbulence_equation_relaxation")),
    _turbulence_field_min_limit(getParam<std::vector<Real>>("turbulence_field_min_limit")),
    _turbulence_l_abs_tol(getParam<Real>("turbulence_l_abs_tol")),
    _solid_energy_absolute_tolerance(getParam<Real>("solid_energy_absolute_tolerance")),
    _turbulence_absolute_tolerance(getParam<std::vector<Real>>("turbulence_absolute_tolerance")),
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

  // We check for input errors with regards to the turbulence equations. At the same time, we
  // set up the corresponding system numbers
  if (_has_turbulence_systems)
  {
    if (_turbulence_system_names.size() != _turbulence_equation_relaxation.size())
      paramError("turbulence_equation_relaxation",
                 "The number of equation relaxation parameters does not match the number of "
                 "turbulence scalar equations!");
    if (_turbulence_system_names.size() != _turbulence_absolute_tolerance.size())
      paramError("turbulence_absolute_tolerance",
                 "The number of absolute tolerances does not match the number of "
                 "turbulence equations!");
    if (_turbulence_field_min_limit.empty())
      // If no minimum bounds are given, initialize to default value 1e-8
      _turbulence_field_min_limit.resize(_turbulence_system_names.size(), 1e-8);
    else if (_turbulence_system_names.size() != _turbulence_field_min_limit.size())
      paramError("turbulence_field_min_limit",
                 "The number of lower bounds for turbulent quantities does not match the "
                 "number of turbulence equations!");
  }

  if (isParamValid("solid_energy_system") && !_has_energy_system)
    paramError(
        "solid_energy_system",
        "We cannot solve a solid energy system without solving for the fluid energy as well!");

  if (_has_energy_system)
  {
    // We only allow the solve for a solid energy system if we already solve for the fluid energy
    if (_has_solid_energy_system)
    {
      const auto & solid_energy_petsc_options =
          getParam<MultiMooseEnum>("solid_energy_petsc_options");
      const auto & solid_energy_petsc_pair_options = getParam<MooseEnumItem, std::string>(
          "solid_energy_petsc_options_iname", "solid_energy_petsc_options_value");
      Moose::PetscSupport::processPetscFlags(solid_energy_petsc_options,
                                             _solid_energy_petsc_options);
      Moose::PetscSupport::processPetscPairs(solid_energy_petsc_pair_options,
                                             _problem.mesh().dimension(),
                                             _solid_energy_petsc_options);

      _solid_energy_linear_control.real_valued_data["rel_tol"] =
          getParam<Real>("solid_energy_l_tol");
      _solid_energy_linear_control.real_valued_data["abs_tol"] =
          getParam<Real>("solid_energy_l_abs_tol");
      _solid_energy_linear_control.int_valued_data["max_its"] =
          getParam<unsigned int>("solid_energy_l_max_its");
    }
    else
      checkDependentParameterError("solid_energy_system",
                                   {"solid_energy_petsc_options",
                                    "solid_energy_petsc_options_iname",
                                    "solid_energy_petsc_options_value",
                                    "solid_energy_l_tol",
                                    "solid_energy_l_abs_tol",
                                    "solid_energy_l_max_its",
                                    "solid_energy_absolute_tolerance"},
                                   false);
  }

  if (_has_turbulence_systems)
  {
    const auto & turbulence_petsc_options = getParam<MultiMooseEnum>("turbulence_petsc_options");
    const auto & turbulence_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "turbulence_petsc_options_iname", "turbulence_petsc_options_value");
    Moose::PetscSupport::processPetscFlags(turbulence_petsc_options, _turbulence_petsc_options);
    Moose::PetscSupport::processPetscPairs(
        turbulence_petsc_pair_options, _problem.mesh().dimension(), _turbulence_petsc_options);

    _turbulence_linear_control.real_valued_data["rel_tol"] = getParam<Real>("turbulence_l_tol");
    _turbulence_linear_control.real_valued_data["abs_tol"] = getParam<Real>("turbulence_l_abs_tol");
    _turbulence_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("turbulence_l_max_its");
  }
  else
    checkDependentParameterError("turbulence_system",
                                 {"turbulence_petsc_options",
                                  "turbulence_petsc_options_iname",
                                  "turbulence_petsc_options_value",
                                  "turbulence_l_tol",
                                  "turbulence_l_abs_tol",
                                  "turbulence_l_max_its",
                                  "turbulence_equation_relaxation",
                                  "turbulence_absolute_tolerance"},
                                 false);
}

void
SIMPLESolveNonlinearAssembly::linkRhieChowUserObject()
{
  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_system_numbers, _pressure_tag_id);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceVelocities();
}

std::vector<std::pair<unsigned int, Real>>
SIMPLESolveNonlinearAssembly::solveMomentumPredictor()
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
    _momentum_systems[system_i]->copyPreviousNonlinearSolutions();
  }

  return its_normalized_residuals;
}

std::pair<unsigned int, Real>
SIMPLESolveNonlinearAssembly::solvePressureCorrector()
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
  Real norm_factor = NS::FV::computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LibmeshPetscCall(KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  _pressure_linear_control.real_valued_data["abs_tol"] = _pressure_l_abs_tol * norm_factor;
  pressure_solver.set_solver_configuration(_pressure_linear_control);

  if (_pin_pressure)
    NS::FV::constrainSystem(mmat, rhs, _pressure_pin_value, _pressure_pin_dof);

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
SIMPLESolveNonlinearAssembly::solveAdvectedSystem(const unsigned int system_num,
                                                  NonlinearSystemBase & system,
                                                  const Real relaxation_factor,
                                                  libMesh::SolverConfiguration & solver_config,
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
SIMPLESolveNonlinearAssembly::solveSolidEnergySystem()
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
  Real norm_factor = NS::FV::computeNormalizationFactor(solution, mat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LibmeshPetscCall(KSPSetNormType(se_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

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

bool
SIMPLESolveNonlinearAssembly::solve()
{
  // Dummy solver parameter file which is needed for switching petsc options
  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

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

  bool converged = false;
  // Loop until converged or hit the maximum allowed iteration number
  while (iteration_counter < _num_iterations && !converged)
  {
    iteration_counter++;
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
    NS::FV::relaxSolutionUpdate(
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
        NS::FV::limitSolutionUpdate(current_solution, _turbulence_field_min_limit[system_i]);

        // Relax the turbulence update for the next momentum predictor
        auto & old_solution = *(_turbulence_systems[system_i]->solutionPreviousNewton());

        // Relax the pressure update for the next momentum predictor
        NS::FV::relaxSolutionUpdate(
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

    converged = NS::FV::converged(ns_its_residuals, ns_abs_tols);
  }

  converged = _continue_on_max_its ? true : converged;

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

    bool passive_scalar_converged =
        NS::FV::converged(passive_scalar_residuals, _passive_scalar_absolute_tolerance);
    while (iteration_counter < _num_iterations && !passive_scalar_converged)
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

      passive_scalar_converged =
          NS::FV::converged(passive_scalar_residuals, _passive_scalar_absolute_tolerance);
    }

    converged = _continue_on_max_its ? true : passive_scalar_converged;
  }

  return converged;
}

void
SIMPLESolveNonlinearAssembly::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  for (const auto system : _momentum_systems)
    checkTimeKernels(*system);

  checkTimeKernels(_pressure_system);

  if (_has_energy_system)
  {
    checkTimeKernels(*_energy_system);
    if (_has_solid_energy_system)
      checkTimeKernels(*_solid_energy_system);
  }

  if (_has_passive_scalar_systems)
    for (const auto system : _passive_scalar_systems)
      checkTimeKernels(*system);

  if (_has_turbulence_systems)
    for (const auto system : _turbulence_systems)
      checkTimeKernels(*system);
}

void
SIMPLESolveNonlinearAssembly::checkTimeKernels(NonlinearSystemBase & system)
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (system.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation in system",
               system.name());
}
