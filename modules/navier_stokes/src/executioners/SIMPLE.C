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
  params.addParam<NonlinearSystemName>(
      "momentum_system", "momentum_system", "The nonlinear system for the momentum equation");
  params.addParam<NonlinearSystemName>(
      "pressure_system", "pressure_system", "The nonlinear system for the pressure equation");
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
    _momentum_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("momentum_system"))),
    _pressure_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("pressure_system"))),
    _momentum_sys(_problem.getNonlinearSystemBase(_momentum_sys_number)),
    _pressure_sys(_problem.getNonlinearSystemBase(_pressure_sys_number)),
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

  _momentum_sys.addVector(_momentum_tag_id, false, ParallelType::PARALLEL);

  _time = 0;
}

void
SIMPLE::init()
{
  _problem.initialSetup();
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_sys, _momentum_tag_id);
  _rc_uo->initFaceVelocities();
}

void
SIMPLE::relaxEquation(SparseMatrix<Number> & matrix_in,
                      NumericVector<Number> & rhs_in,
                      NumericVector<Number> & solution_in,
                      const Real relaxation_parameter)
{
  PetscVector<Number> * solution = cast_ptr<PetscVector<Number> *>(&solution_in);
  PetscVector<Number> * rhs = cast_ptr<PetscVector<Number> *>(&rhs_in);
  PetscMatrix<Number> * matrix = cast_ptr<PetscMatrix<Number> *>(&matrix_in);

  std::unique_ptr<NumericVector<Number>> working_vector = solution->zero_clone();
  PetscVector<Number> * working_vector_petsc =
      dynamic_cast<PetscVector<Number> *>(working_vector.get());

  matrix->get_diagonal(*working_vector_petsc);

  std::unique_ptr<NumericVector<Number>> original_diagonal = working_vector_petsc->clone();
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
    Real abs_diagonal = std::abs((*working_vector_petsc)(row_i));
    Real new_diagonal = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    working_vector_petsc->set(row_i, new_diagonal);
  }

  for (auto row_i = matrix->row_start(); row_i < matrix->row_stop(); row_i++)
  {
    matrix->set(row_i, row_i, (*working_vector_petsc)(row_i));
  }
  matrix->close();

  working_vector_petsc->add(-1.0, *original_diagonal_petsc);
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

Real
SIMPLE::solveMomentumPredictor(NonlinearImplicitSystem & momentum_system)
{
  _problem.setCurrentNonlinearSystem(_momentum_sys_number);
  PetscLinearSolver<Real> & momentum_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());

  // KSP momentum_solve.ksp();

  auto zero_solution = momentum_system.current_local_solution->zero_clone();
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);

  _problem.computeResidualAndJacobian(*zero_solution, *momentum_system.rhs, *mmat);

  momentum_system.rhs->scale(-1.0);

  relaxEquation(*mmat,
                *momentum_system.rhs,
                *momentum_system.current_local_solution,
                getMomentumRelaxation());

  momentum_solver.solve(
      *mmat, *mmat, *momentum_system.current_local_solution, *momentum_system.rhs, 1e-8, 100);

  if (_print_fields)
  {
    std::cout << " rhs when we solve " << std::endl;
    momentum_system.rhs->print();
  }

  std::vector<Real> res_history;
  momentum_solver.get_residual_history(res_history);
  if (_print_fields)
    std::cout << Moose::stringify(res_history) << std::endl;
  return res_history[0];
}

Real
SIMPLE::solvePressureCorrector(NonlinearImplicitSystem & pressure_system)
{
  _problem.setCurrentNonlinearSystem(_pressure_sys_number);

  PetscVector<Number> * pressure_solution =
      dynamic_cast<PetscVector<Number> *>(pressure_system.current_local_solution.get());

  PetscLinearSolver<Real> & pressure_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  // KSP momentum_solve.ksp();

  auto zero_solution = pressure_solution->zero_clone();
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);

  _problem.computeResidualAndJacobian(*zero_solution, *pressure_system.rhs, *mmat);

  if (_print_fields)
  {
    std::cout << "Pressure matrix" << std::endl;
    mmat->print();
  }

  pressure_system.rhs->scale(-1.0);

  pressure_solver.solve(*mmat, *mmat, *pressure_solution, *pressure_system.rhs, 1e-8, 100);

  if (_print_fields)
  {
    std::cout << " rhs when we solve pressure " << std::endl;
    pressure_system.rhs->print();
  }

  _pressure_sys.setSolution(*pressure_solution);
  std::vector<Real> res_history;
  pressure_solver.get_residual_history(res_history);
  if (_print_fields)
    std::cout << Moose::stringify(res_history) << std::endl;
  return res_history[0];
}

void
SIMPLE::relaxPressureUpdate(NonlinearImplicitSystem & pressure_system)
{
  PetscVector<Number> * pressure_solution =
      dynamic_cast<PetscVector<Number> *>(pressure_system.current_local_solution.get());
  PetscVector<Number> * pressure_solution_old =
      dynamic_cast<PetscVector<Number> *>(_pressure_sys.solutionPreviousNewton());

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
  _pressure_sys.setSolution(*pressure_solution);
  _pressure_sys.residualSetup();
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

  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(_momentum_sys.system());
  NonlinearImplicitSystem & pressure_system =
      dynamic_cast<NonlinearImplicitSystem &>(_pressure_sys.system());

  PetscErrorCode ierr;

  PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);
  PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);

  unsigned int iteration_counter = 0;
  Real momentum_residual = 1.0;
  Real pressure_residual = 1.0;
  while (iteration_counter < _num_iterations && (momentum_residual > _momentum_absolute_tolerance &&
                                                 pressure_residual > _pressure_absolute_tolerance))
  {
    Moose::PetscSupport::petscSetOptions(_problem);
    Moose::setSolverDefaults(_problem);
    _momentum_sys.residualSetup();
    _pressure_sys.residualSetup();

    iteration_counter++;

    momentum_residual = solveMomentumPredictor(momentum_system);

    _rc_uo->computeHbyA(_momentum_equation_relaxation, _print_fields);

    pressure_residual = solvePressureCorrector(pressure_system);

    _rc_uo->computeFaceVelocity();

    relaxPressureUpdate(pressure_system);

    _rc_uo->computeCellVelocity();

    _console << "Iteration " << iteration_counter << " Initial residual norms:\n"
             << " Momentum equation: " << COLOR_GREEN << momentum_residual << COLOR_DEFAULT << "\n"
             << " Pressure equation: " << COLOR_GREEN << pressure_residual << COLOR_DEFAULT
             << std::endl;
  }
}
