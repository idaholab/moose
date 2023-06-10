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
  params.addParam<std::string>("momentum_tag",
                               "non_pressure_momentum_kernels",
                               "The name of the tags associated with the kernels in the momentum "
                               "equations which are not related to the pressure gradient.");
  params.addRangeCheckedParam<Real>(
      "momentum_variable_relaxation",
      1.0,
      "0.0<momentum_variable_relaxation<=1.0",
      "The relaxation which should be used for the momentum variable.");
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
      "pressure_equation_relaxation",
      1.0,
      "0.0<pressure_equation_relaxation<=1.0",
      "The relaxation which should be used for the pressure equation.");
  params.addRangeCheckedParam<Real>(
      "momentum_absolute_tolerance",
      1e-5,
      "0.0<momentum_absolute_tolerance",
      "The absolute tolerance on ther residual of the momentum equation.");
  params.addRangeCheckedParam<Real>(
      "pressure_absolute_tolerance",
      1e-5,
      "0.0<pressure_absolute_tolerance",
      "The absolute tolerance on ther residual of the pressure equation.");
  params.addRangeCheckedParam<unsigned int>("num_iterations",
                                            1000,
                                            "0<num_iterations",
                                            "The number of momentum-pressure iterations needed.");
  params.addParam<bool>("print_fields", false, "ASD");

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
    _momentum_tag_name(getParam<std::string>("momentum_tag")),
    _momentum_tag_id(_problem.addVectorTag(_momentum_tag_name)),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations")),
    _print_fields(getParam<bool>("print_fields"))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.nlSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(
        &_problem.getNonlinearSystemBase(_momentum_system_numbers[system_i]));
    _momentum_systems[system_i]->addVector(_momentum_tag_id, false, ParallelType::PARALLEL);
  }

  _time = 0;
}

void
SIMPLE::init()
{
  _problem.initialSetup();
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_tag_id);
  _rc_uo->initFaceVelocities();
}

void
SIMPLE::relaxMatrix(SparseMatrix<Number> & matrix_in,
                    const Real relaxation_parameter,
                    NumericVector<Number> & diff_diagonal)
{
  PetscMatrix<Number> * matrix = cast_ptr<PetscMatrix<Number> *>(&matrix_in);

  diff_diagonal = 0;

  matrix->get_diagonal(diff_diagonal);

  std::unique_ptr<NumericVector<Number>> original_diagonal = diff_diagonal.clone();
  PetscVector<Number> * original_diagonal_petsc =
      dynamic_cast<PetscVector<Number> *>(original_diagonal.get());

  const Real inverse_relaxation = 1 / relaxation_parameter;
  for (auto row_i = matrix->row_start(); row_i < matrix->row_stop(); row_i++)
  {
    std::vector<numeric_index_type> indices;
    std::vector<Real> values;
    matrix->get_row(row_i, indices, values);
    Real abs_sum = std::transform_reduce(
        values.cbegin(), values.cend(), 0.0, std::plus{}, [](auto val) { return std::abs(val); });
    Real abs_diagonal = std::abs(diff_diagonal(row_i));
    Real new_diagonal = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    diff_diagonal.set(row_i, new_diagonal);
  }

  for (auto row_i = matrix->row_start(); row_i < matrix->row_stop(); row_i++)
  {
    matrix->set(row_i, row_i, diff_diagonal(row_i));
  }
  matrix->close();

  diff_diagonal.add(-1.0, *original_diagonal_petsc);
}

void
SIMPLE::relaxRightHandSide(NumericVector<Number> & rhs_in,
                           const NumericVector<Number> & solution_in,
                           const NumericVector<Number> & diff_diagonal)
{
  const PetscVector<Number> * const solution =
      dynamic_cast<const PetscVector<Number> * const>(&solution_in);
  PetscVector<Number> * rhs = dynamic_cast<PetscVector<Number> *>(&rhs_in);
  const PetscVector<Number> * const diff_digaonal_petsc =
      dynamic_cast<const PetscVector<Number> * const>(&diff_diagonal);

  // diff_digaonal_petsc.close();
  auto working_vector = diff_digaonal_petsc->clone();
  PetscVector<Number> * working_vector_petsc =
      dynamic_cast<PetscVector<Number> *>(working_vector.get());

  working_vector_petsc->pointwise_mult(*solution, *working_vector_petsc);

  // if (_print_fields)
  // {
  //   std::cout << " rhs before relaxation " << std::endl;
  //   rhs->print();
  //   std::cout << " relaxation source " << std::endl;
  //   working_vector_petsc->print();
  // }

  rhs->add(*working_vector_petsc);
  rhs->close();

  // if (_print_fields)
  // {
  //   std::cout << " rhs after relaxation " << std::endl;
  //   rhs->print();
  // }
}

std::vector<Real>
SIMPLE::solveMomentumPredictor(std::vector<NonlinearSystemBase *> & momentum_systems)
{
  std::vector<Real> normalized_residuals;
  _problem.setCurrentNonlinearSystem(_momentum_system_numbers[0]);

  NonlinearImplicitSystem * momentum_system =
      dynamic_cast<NonlinearImplicitSystem *>(&(_momentum_systems[0]->system()));

  PetscLinearSolver<Real> & momentum_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*momentum_system->get_linear_solver());

  auto working_vector = momentum_system->current_local_solution->zero_clone();
  auto diff_diagonal = momentum_system->current_local_solution->zero_clone();
  PetscVector<Number> * solution =
      dynamic_cast<PetscVector<Number> *>(momentum_system->current_local_solution.get());
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
  PetscVector<Number> * rhs = dynamic_cast<PetscVector<Number> *>(momentum_system->rhs);

  _problem.computeResidualAndJacobian(*working_vector, *rhs, *mmat);

  rhs->scale(-1.0);

  relaxMatrix(*mmat, getMomentumRelaxation(), *diff_diagonal);

  diff_diagonal->close();

  Real norm_factor = computeNormalizationFactor(*solution, *mmat, *rhs);
  Real absolute_tolerance = norm_factor * 1e-8;

  KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  momentum_solver.solve(*mmat, *mmat, *solution, *rhs, absolute_tolerance, 100);

  if (_print_fields)
  {
    std::cout << " rhs when we solve " << std::endl;
    rhs->print();
  }

  if (_print_fields)
  {
    std::cout << "Norm factor " << norm_factor << std::endl;
    std::cout << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
  }
  std::cout << "Norm factor " << norm_factor << std::endl;

  normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

  for (auto system_i : index_range(_momentum_systems))
  {
    if (!system_i)
      continue;

    _problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    momentum_system =
        dynamic_cast<NonlinearImplicitSystem *>(&(_momentum_systems[system_i]->system()));

    PetscVector<Number> * solution =
        dynamic_cast<PetscVector<Number> *>(momentum_system->current_local_solution.get());
    PetscVector<Number> * rhs = dynamic_cast<PetscVector<Number> *>(momentum_system->rhs);

    _problem.computeResidual(*working_vector, *rhs);

    rhs->scale(-1.0);

    relaxRightHandSide(*rhs, *solution, *diff_diagonal);

    norm_factor = computeNormalizationFactor(*solution, *mmat, *rhs);
    absolute_tolerance = norm_factor * 1e-8;

    momentum_solver.solve(*mmat, *mmat, *solution, *rhs, absolute_tolerance, 100);

    normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

    if (_print_fields)
    {
      std::cout << " rhs when we solve " << std::endl;
      rhs->print();
    }

    if (_print_fields)
    {
      std::cout << "Norm factor " << norm_factor << std::endl;
      std::cout << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
    }
    std::cout << "Norm factor " << norm_factor << std::endl;
  }

  return normalized_residuals;
}

PetscReal
SIMPLE::computeNormalizationFactor(const PetscVector<Number> & solution,
                                   const PetscMatrix<Number> & mat,
                                   const PetscVector<Number> & rhs)
{
  auto average_solution = solution.zero_clone();
  auto multiplied_solution = solution.zero_clone();

  *average_solution = 1.0;
  *multiplied_solution = solution.sum() / solution.size();

  if (_print_fields)
  {
    std::cout << "Average solution " << solution.sum() / solution.size() << std::endl;
  }

  // Beware, trickey here!
  mat.vector_mult(*average_solution, *multiplied_solution);
  mat.vector_mult(*multiplied_solution, solution);

  multiplied_solution->add(-1.0, *average_solution);
  average_solution->add(-1.0, rhs);

  multiplied_solution->abs();
  average_solution->abs();

  average_solution->add(*multiplied_solution);

  return average_solution->l2_norm();
}

Real
SIMPLE::solvePressureCorrector(NonlinearSystemBase & pressure_system_in)
{
  _problem.setCurrentNonlinearSystem(_pressure_sys_number);

  NonlinearImplicitSystem & pressure_system =
      dynamic_cast<NonlinearImplicitSystem &>(pressure_system_in.system());
  PetscVector<Number> * solution =
      dynamic_cast<PetscVector<Number> *>(pressure_system.current_local_solution.get());
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);
  PetscVector<Number> * rhs = dynamic_cast<PetscVector<Number> *>(pressure_system.rhs);

  PetscLinearSolver<Real> & pressure_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  // KSP momentum_solve.ksp();

  auto zero_solution = solution->zero_clone();

  _problem.computeResidualAndJacobian(*zero_solution, *rhs, *mmat);

  if (_print_fields)
  {
    std::cout << "Pressure matrix" << std::endl;
    mmat->print();
  }

  rhs->scale(-1.0);

  Real norm_factor = computeNormalizationFactor(*solution, *mmat, *rhs);
  Real absolute_tolerance = norm_factor * 1e-8;

  KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED);
  pressure_solver.solve(*mmat, *mmat, *solution, *rhs, absolute_tolerance, 100);

  if (_print_fields)
  {
    std::cout << " rhs when we solve pressure " << std::endl;
    rhs->print();
  }

  _pressure_system.setSolution(*solution);
  if (_print_fields)
  {
    std::cout << pressure_solver.get_initial_residual() << std::endl;
    std::cout << "Norm factor " << norm_factor << std::endl;
  }
  std::cout << "Norm factor " << norm_factor << std::endl;
  return pressure_solver.get_initial_residual() / norm_factor;
}

void
SIMPLE::relaxPressureUpdate(NonlinearSystemBase & pressure_system_in)
{
  NonlinearImplicitSystem & pressure_system =
      dynamic_cast<NonlinearImplicitSystem &>(pressure_system_in.system());
  PetscVector<Number> * pressure_solution =
      dynamic_cast<PetscVector<Number> *>(pressure_system.current_local_solution.get());
  PetscVector<Number> * pressure_solution_old =
      dynamic_cast<PetscVector<Number> *>(_pressure_system.solutionPreviousNewton());

  pressure_solution->scale(_pressure_variable_relaxation);
  pressure_solution->add(1 - _pressure_variable_relaxation, *pressure_solution_old);

  // solution->print();
  pressure_solution->close();

  if (_print_fields)
  {
    std::cout << "Pressure solution" << std::endl;
    pressure_solution->print();
    std::cout << "Pressure solution old" << std::endl;
    pressure_solution_old->print();
  }

  *pressure_solution_old = *pressure_solution;
  _pressure_system.setSolution(*pressure_solution);
  _pressure_system.residualSetup();
}

void
SIMPLE::execute()
{
  _problem.timestepSetup();

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  Moose::PetscSupport::PetscOptions & po = _problem.getPetscOptions();

  unsigned int iteration_counter = 0;
  std::vector<Real> momentum_residual(1.0, _momentum_systems.size());
  Real pressure_residual = 1.0;
  while (iteration_counter < _num_iterations && !converged(momentum_residual, pressure_residual))
  {
    Moose::PetscSupport::petscSetOptions(_problem);
    Moose::setSolverDefaults(_problem);
    for (auto system_i : index_range(_momentum_systems))
      _momentum_systems[system_i]->residualSetup();
    _pressure_system.residualSetup();

    iteration_counter++;

    momentum_residual = solveMomentumPredictor(_momentum_systems);

    _rc_uo->computeHbyA(_momentum_equation_relaxation, _print_fields);

    pressure_residual = solvePressureCorrector(_pressure_system);

    _rc_uo->computeFaceVelocity();

    relaxPressureUpdate(_pressure_system);

    _rc_uo->computeCellVelocity();

    _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
    for (auto system_i : index_range(_momentum_systems))
    {
      _console << " Momentum equation: "
               << (_momentum_systems.size() > 1
                       ? std::string("Component ") + std::to_string(system_i + 1)
                       : std::string(""))
               << COLOR_GREEN << momentum_residual[system_i] << COLOR_DEFAULT << std::endl;
    }
    _console << " Pressure equation: " << COLOR_GREEN << pressure_residual << COLOR_DEFAULT
             << std::endl;
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
