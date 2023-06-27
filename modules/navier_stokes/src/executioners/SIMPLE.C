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
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = Executioner::validParams();
  params += FEProblemSolve::validParams();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  params.addRequiredParam<std::vector<std::string>>(
      "momentum_systems", "The nonlinear system for the momentum equation");
  params.addRequiredParam<NonlinearSystemName>("pressure_system",
                                               "The nonlinear system for the pressure equation");
  params.addParam<std::string>("pressure_gradient_tag",
                               "non_pressure_momentum_kernels",
                               "The name of the tags associated with the kernels in the momentum "
                               "equations which are not related to the pressure gradient.");
  params.addRangeCheckedParam<Real>(
      "pressure_variable_relaxation",
      1.0,
      "0.0<pressure_variable_relaxation<=1.0",
      "The relaxation which should be used for the pressure variable.");
  params.addRangeCheckedParam<Real>(
      "momentum_equation_relaxation",
      1.0,
      "0.0<momentum_equation_relaxation<=1.0",
      "The relaxation which should be used for the momentum equation.");
  params.addRangeCheckedParam<Real>(
      "momentum_absolute_tolerance",
      1e-5,
      "0.0<momentum_absolute_tolerance",
      "The absolute tolerance on the residual of the momentum equation.");
  params.addRangeCheckedParam<Real>(
      "pressure_absolute_tolerance",
      1e-5,
      "0.0<pressure_absolute_tolerance",
      "The absolute tolerance on ther residual of the pressure equation.");
  params.addRangeCheckedParam<unsigned int>("num_iterations",
                                            1000,
                                            "0<num_iterations",
                                            "The number of momentum-pressure iterations needed.");
  params.addParam<bool>(
      "print_fields",
      false,
      "Use this to print the coupling and solution fields and matrices throughout the iteration.");

  return params;
}

SIMPLE::SIMPLE(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _momentum_system_names(getParam<std::vector<std::string>>("momentum_systems")),
    _pressure_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("pressure_system"))),
    _pressure_system(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _pressure_tag_name(getParam<std::string>("pressure_gradient_tag")),
    _pressure_tag_id(_problem.addVectorTag(_pressure_tag_name)),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations")),
    _print_fields(getParam<bool>("print_fields"))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  if (_momentum_system_names.size() != 1 &&
      _momentum_system_names.size() != _problem.mesh().dimension())
    paramError("momentum_systems",
               "The number of momentum components should either be equal to 1 or the number of "
               "spatial dimensions on the mesh.");

  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.nlSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(
        &_problem.getNonlinearSystemBase(_momentum_system_numbers[system_i]));
    _momentum_systems[system_i]->addVector(_pressure_tag_id, false, ParallelType::PARALLEL);
  }

  _time = 0;
}

void
SIMPLE::init()
{
  _problem.initialSetup();

  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_system_numbers, _pressure_tag_id);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceVelocities();
}

void
SIMPLE::relaxMatrix(SparseMatrix<Number> & matrix,
                    const Real relaxation_parameter,
                    NumericVector<Number> & diff_diagonal)
{
  // Zero the diagonal difference vector
  diff_diagonal = 0;

  // Get the diagonal of the matrix
  matrix.get_diagonal(diff_diagonal);

  // Create a copy of the diagonal for later use and cast it
  std::unique_ptr<NumericVector<Number>> original_diagonal = diff_diagonal.clone();

  // We cache the inverse of the relaxation parameter because doing divisions might
  // be more expensive for every row
  const Real inverse_relaxation = 1 / relaxation_parameter;

  // Now we loop over the matrix row by row and sum the absolute values of the
  // offdiagonal values, If these sums are larger than the diagonal entry,
  // we switch the diagonal value with the sum. At the same time we increase the
  // diagonal by dividing it with the relaxation parameter. So the new diagonal will be:
  // D* = 1/lambda*max(|D|,sum(|Offdiag|))
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  //
  // The trickery comes with storing everything in the diff-diagonal vector
  // to avoid the allocation and manipulation of a third vector
  for (auto row_i = matrix.row_start(); row_i < matrix.row_stop(); row_i++)
  {
    std::vector<numeric_index_type> indices;
    std::vector<Real> values;
    matrix.get_row(row_i, indices, values);
    Real abs_sum = std::transform_reduce(
        values.cbegin(), values.cend(), 0.0, std::plus{}, [](auto val) { return std::abs(val); });
    Real abs_diagonal = std::abs(diff_diagonal(row_i));
    Real new_diagonal = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    diff_diagonal.set(row_i, new_diagonal);
  }

  // Time to modify the diagonal of the matrix
  for (auto row_i = matrix.row_start(); row_i < matrix.row_stop(); row_i++)
    matrix.set(row_i, row_i, diff_diagonal(row_i));
  matrix.close();

  // Finally, we can create (D*-D) vector which is used for the relaxation of the
  // right hand side later
  diff_diagonal.add(-1.0, *original_diagonal);
  diff_diagonal.close();
}

void
SIMPLE::relaxRightHandSide(NumericVector<Number> & rhs,
                           const NumericVector<Number> & solution,
                           const NumericVector<Number> & diff_diagonal)
{

  // We need a working vector here to make sure we don't modify the
  // (D*-D) vector
  auto working_vector = diff_diagonal.clone();
  working_vector->pointwise_mult(solution, *working_vector);

  // The correction to the right hand side is just
  // (D*-D)*old_solution
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  rhs.add(*working_vector);
  rhs.close();
}

std::vector<Real>
SIMPLE::solveMomentumPredictor()
{
  // Temporary storage for the (flux-normalized) residuals form
  // different momentum components
  std::vector<Real> normalized_residuals;
  _problem.setCurrentNonlinearSystem(_momentum_system_numbers[0]);

  // We will use functions from the implicit system directly
  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(_momentum_systems[0]->system());

  // We need a linear solver
  // TODO: ADD FUNCTIONALITY TO LIBMESH TO ACCEPT ABS TOLERANCES!
  PetscLinearSolver<Real> & momentum_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());

  // We create a vector which can be used as a helper in different situations. We
  // need the ghosting here so we use current_local_solution
  auto zero_solution = momentum_system.current_local_solution->zero_clone();

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = momentum_system.solution->zero_clone();

  // We will use the solution vector of the equation
  NumericVector<Number> & solution = *(momentum_system.solution);

  // We get the matrix and right hand side
  SparseMatrix<Number> & mmat = *(momentum_system.matrix);
  NumericVector<Number> & rhs = *(momentum_system.rhs);

  // We plug zero in this to get the system matrix and the right hand side of the linear problem
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);

  // Unfortunately, the the right hand side has the opposite side due to the definition of the
  // residual
  rhs.scale(-1.0);

  // Go and relax the system matrix and the right hand side
  relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
  relaxRightHandSide(rhs, solution, *diff_diagonal);

  // We need to compute normalization factors to be able to decide if we are converged
  // or not
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // Very important, for deciding the convergence, we need the unpreconditioned
  // norms in the linear solve
  KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // We solve the equation
  // TO DO: Add options to this function in Libmesh to accept absolute tolerance
  momentum_solver.solve(mmat, mmat, solution, rhs, 1e-10, 100);
  momentum_system.update();
  // Make sure that we reuse the preconditioner if we end up solving the segregated
  // momentum components
  momentum_solver.reuse_preconditioner(true);

  if (_print_fields)
  {
    std::cout << " matrix when we solve " << std::endl;
    mmat.print();
    std::cout << " rhs when we solve " << std::endl;
    rhs.print();
    std::cout << " velocity solution component 0" << std::endl;
    solution.print();
    std::cout << "Norm factor " << norm_factor << std::endl;
    std::cout << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
  }

  // Compute the normalized residual
  normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

  // If we use a segregated approach between momentum components as well, we need to solve
  // the other equations. Luckily, the system matrix is exactly the same so we only need
  // to compute right hand sides.
  for (auto system_i : index_range(_momentum_systems))
  {
    // We are already done with the first system
    if (!system_i)
      continue;

    _problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    // We will need the right hand side and the solution of the next component
    NonlinearImplicitSystem & momentum_system =
        dynamic_cast<NonlinearImplicitSystem &>(_momentum_systems[system_i]->system());
    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);

    // Only evaluating right hand side which is R(0)
    _problem.computeResidual(*zero_solution, rhs, _momentum_system_numbers[system_i]);
    // Sadly, this returns -b so we multiply with -1
    rhs.scale(-1.0);

    // Still need to relax the right hand side with the same vector
    relaxRightHandSide(rhs, solution, *diff_diagonal);

    // The normalization factor depends on the right hand side so we need to recompute it for this
    // component
    norm_factor = computeNormalizationFactor(solution, mmat, rhs);

    // Solve this component
    momentum_solver.solve(mmat, mmat, solution, rhs, 1e-10, 100);
    momentum_system.update();
    // Save the normalized residual
    normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

    if (_print_fields)
    {
      std::cout << " rhs when we solve " << std::endl;
      rhs.print();
      std::cout << " velocity solution component " << system_i << std::endl;
      solution.print();
      std::cout << "Norm factor " << norm_factor << std::endl;
      std::cout << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
    }
  }

  return normalized_residuals;
}

PetscReal
SIMPLE::computeNormalizationFactor(const NumericVector<Number> & solution,
                                   const SparseMatrix<Number> & mat,
                                   const NumericVector<Number> & rhs)
{
  // This function is based on the description provided here:
  // https://www.openfoam.com/documentation/guides/latest/doc/guide-solvers-residuals.html
  // (Accessed 06/01/2023)
  // so basically we normalize the residual with the following number:
  // sum(|Ax-Ax_avg|+|b-Ax_avg|)
  // where A is the system matrix, b is the system right hand side while x and x_avg are
  // the solution and average solution vectors

  // We create a vector for Ax_avg and Ax
  auto A_times_average_solution = solution.zero_clone();
  auto A_times_solution = solution.zero_clone();

  // Beware, trickery here! To avoid allocating unused vectors, we
  // first compute Ax_avg using the storage used for Ax, then we
  // overwrite Ax with the right value
  *A_times_solution = solution.sum() / solution.size();
  mat.vector_mult(*A_times_average_solution, *A_times_solution);
  mat.vector_mult(*A_times_solution, solution);

  // We create Ax-Ax_avg
  A_times_solution->add(-1.0, *A_times_average_solution);
  // We create Ax_avg - b (ordering shouldn't matter we will take absolute value soon)
  A_times_average_solution->add(-1.0, rhs);
  A_times_solution->abs();
  A_times_average_solution->abs();

  // Create |Ax-Ax_avg|+|b-Ax_avg|
  A_times_average_solution->add(*A_times_solution);

  // Since use the l2 norm of the solution vectors in the linear solver, we will
  // make this consistent and use the l2 norm of the vector
  // TODO: Would be nice to see if we can do l1 norms in the linear solve
  return A_times_average_solution->l2_norm();
}

Real
SIMPLE::solvePressureCorrector()
{
  _problem.setCurrentNonlinearSystem(_pressure_sys_number);

  // We will need some members from the implocot nonlinear system
  NonlinearImplicitSystem & pressure_system =
      dynamic_cast<NonlinearImplicitSystem &>(_pressure_system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(pressure_system.current_local_solution);
  NumericVector<Number> & solution = *(pressure_system.solution);
  SparseMatrix<Number> & mmat = *(pressure_system.matrix);
  NumericVector<Number> & rhs = *(pressure_system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & pressure_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the righ hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
  rhs.scale(-1.0);

  if (_print_fields)
  {
    std::cout << "Pressure matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // Solve the system and update current local solution
  pressure_solver.solve(mmat, mmat, solution, rhs, 1e-10, 100);
  pressure_system.update();

  if (_print_fields)
  {
    std::cout << " rhs when we solve pressure " << std::endl;
    rhs.print();
    std::cout << " Pressure " << std::endl;
    solution.print();
    std::cout << "Norm factor " << norm_factor << std::endl;
  }

  _pressure_system.setSolution(current_local_solution);

  return pressure_solver.get_initial_residual() / norm_factor;
}

void
SIMPLE::relaxSolutionUpdate(NonlinearSystemBase & system_in, Real relaxation_factor)
{
  // We will need the latest and the second latest solution for the relaxation
  NonlinearImplicitSystem & system = dynamic_cast<NonlinearImplicitSystem &>(system_in.system());
  NumericVector<Number> & solution = *(system.current_local_solution.get());
  NumericVector<Number> & solution_old = *(system_in.solutionPreviousNewton());

  // The relaxation is just u = lambda * u* + (1-lambda) u_old
  solution.scale(relaxation_factor);
  solution.add(1 - relaxation_factor, solution_old);
  solution.close();

  if (_print_fields)
  {
    std::cout << "Pressure solution" << std::endl;
    solution.print();
    std::cout << "Pressure solution old" << std::endl;
    solution_old.print();
  }

  // We will overwrite the old solution here
  solution_old = solution;
  system_in.setSolution(solution);
  system_in.residualSetup();
}

void
SIMPLE::execute()
{
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
    // Initialize the quantities which matter in terms of the iteration
    unsigned int iteration_counter = 0;
    std::vector<Real> momentum_residual(1.0, _momentum_systems.size());
    Real pressure_residual = 1.0;

    // Loop until converged or hit the maximum allowed iteration number
    while (iteration_counter < _num_iterations && !converged(momentum_residual, pressure_residual))
    {
      // We set the preconditioner type using this option.
      // TODO: We need a way to specify different perconditioners for different systems
      Moose::PetscSupport::petscSetOptions(_problem);
      Moose::setSolverDefaults(_problem);

      // We clear the caches in the momentum and pressure variables
      for (auto system_i : index_range(_momentum_systems))
        _momentum_systems[system_i]->residualSetup();
      _pressure_system.residualSetup();

      iteration_counter++;

      // Solve the momentum predictor step
      momentum_residual = solveMomentumPredictor();

      // Compute the coupling fields between the momentum and pressure equations
      _rc_uo->computeHbyA(_print_fields);

      // Solve the pressure corrector
      pressure_residual = solvePressureCorrector();

      // Compute the face velocity which is used in the advection terms
      _rc_uo->computeFaceVelocity();

      // Relax the pressure update for the next momentum predictor
      relaxSolutionUpdate(_pressure_system, _pressure_variable_relaxation);

      // Reconstruct the cell velocity as well to accelerate convergence
      _rc_uo->computeCellVelocity();

      _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
      for (auto system_i : index_range(_momentum_systems))
        _console << " Momentum equation: "
                 << (_momentum_systems.size() > 1
                         ? std::string("Component ") + std::to_string(system_i + 1)
                         : std::string(""))
                 << COLOR_GREEN << momentum_residual[system_i] << COLOR_DEFAULT << std::endl;
      _console << " Pressure equation: " << COLOR_GREEN << pressure_residual << COLOR_DEFAULT
               << std::endl;
    }
  }

  _time = _time_step;
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
  }
}

bool
SIMPLE::converged(const std::vector<Real> & momentum_residuals, const Real pressure_residual)
{
  bool converged = true;
  for (const auto & residual : momentum_residuals)
  {
    converged = converged && (residual < _momentum_absolute_tolerance);
    if (!converged)
      return converged;
  }
  converged = converged && (pressure_residual < _pressure_absolute_tolerance);
  return converged;
}
