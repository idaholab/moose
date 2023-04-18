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

#include "libmesh/equation_systems.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
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
      0.7,
      "0.0<momentum_variable_relaxation<=1.0",
      "The relaxation which should be used for the momentum variable.");
  params.addRangeCheckedParam<Real>(
      "pressure_variable_relaxation",
      0.5,
      "0.0<pressure_variable_relaxation<=1.0",
      "The relaxation which should be used for the pressure variable.");
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
    _momentum_variable_relaxation(getParam<Real>("momentum_variable_relaxation")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations"))
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

PetscErrorCode
SIMPLE::relaxation_stuff(KSP ksp, Vec rhs, Vec x, void * ctx)
{
  SIMPLE * executioner = static_cast<SIMPLE *>(ctx);

  NonlinearImplicitSystem & momentum_system =
      dynamic_cast<NonlinearImplicitSystem &>(executioner->getMomentumSystem().system());

  PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);

  PetscVector<Number> * sys_rhs = dynamic_cast<PetscVector<Number> *>(momentum_system.rhs);

  PetscVector<Number> * solution =
      dynamic_cast<PetscVector<Number> *>(momentum_system.solution.get());

  PetscVector<Number> * old_solution = dynamic_cast<PetscVector<Number> *>(
      &executioner->getMomentumSystem().getVector(Moose::PREVIOUS_NL_SOLUTION_TAG));

  std::cout << " IN KSP PRESOLVE: " << std::endl;
  std::cout << " Old " << std::endl;
  old_solution->print();
  std::cout << " New " << std::endl;
  solution->print();


  std::unique_ptr<NumericVector<Number>> working_vector = old_solution->zero_clone();
  PetscVector<Number> * working_vector_petsc =
      dynamic_cast<PetscVector<Number> *>(working_vector.get());

  std::unique_ptr<NumericVector<Number>> diff_vector = solution->clone();
  PetscVector<Number> * diff_vector_petsc = dynamic_cast<PetscVector<Number> *>(diff_vector.get());

  pmat->get_diagonal(*working_vector_petsc);

  working_vector_petsc->scale(1.0 / executioner->getMomentumRelaxation());

  for (auto row_i = pmat->row_start(); row_i < pmat->row_stop(); row_i++)
  {
    pmat->set(row_i, row_i, (*working_vector_petsc)(row_i));
  }
  pmat->close();

//   diff_vector_petsc->add(-1.0, *old_solution);
//   diff_vector_petsc->pointwise_mult(*diff_vector_petsc, *working_vector_petsc);
//   diff_vector_petsc->scale(executioner->getMomentumRelaxation() - 1);

//   sys_rhs->add(-1.0, *diff_vector_petsc);
//   sys_rhs->close();

  return 0;
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
  PetscLinearSolver<Real> & momentum_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());
  NonlinearImplicitSystem & pressure_system =
      dynamic_cast<NonlinearImplicitSystem &>(_pressure_sys.system());
  PetscLinearSolver<Real> & pressure_solver =
      dynamic_cast<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  unsigned int iteration_counter = 0;
  _problem.computeResidualSys(
      momentum_system, *_momentum_sys.currentSolution(), *momentum_system.rhs);
  Real momentum_residual = 1.0;
  Real pressure_residual = 1.0;

  PetscNonlinearSolver<Real> * solver =
      static_cast<PetscNonlinearSolver<Real> *>(momentum_system.nonlinear_solver.get());

  PetscErrorCode ierr;

  PetscVector<Number> * pressure_solution =
        dynamic_cast<PetscVector<Number> *>(pressure_system.solution.get());

  PetscVector<Number> * pressure_solution_old = dynamic_cast<PetscVector<Number> *>(
        _pressure_sys.solutionPreviousNewton());

  PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);


  while (iteration_counter < _num_iterations && (momentum_residual > _momentum_absolute_tolerance &&
                                                 pressure_residual > _pressure_absolute_tolerance))
  {
    KSP linear_system;
    ierr = SNESGetKSP(solver->snes(), &linear_system);
    LIBMESH_CHKERR(ierr);

    _momentum_sys.residualSetup();
    _pressure_sys.residualSetup();

    ierr = KSPSetPreSolve(linear_system, this->relaxation_stuff, this);
    LIBMESH_CHKERR(ierr);
    iteration_counter++;
    // if (iteration_counter)
    // {
    //   _problem.computeResidualSys(
    //       momentum_system, *_momentum_sys.currentSolution(), *momentum_system.rhs);
    //   momentum_residual = momentum_system.rhs->l2_norm();
    // }

    // bool found = false;
    // for (unsigned int j = 0; j < po.pairs.size(); j++)
    //   if (po.pairs[j].first == MooseUtils::toUpper("-snes_linesearch_damping"))
    //   {
    //     po.pairs[j].second = std::to_string(_momentum_variable_relaxation);
    //     found = true;
    //     break;
    //   }

    // if (!found)
    //   po.pairs.emplace_back(std::make_pair(MooseUtils::toUpper("-snes_linesearch_damping"),
    //                                        std::to_string(_momentum_variable_relaxation)));

    // Moose::PetscSupport::petscSetOptions(_problem);

    _problem.solve(_momentum_sys_number);

    momentum_residual = _momentum_sys._initial_residual_after_preset_bcs;

    _rc_uo->computeHbyA(_momentum_variable_relaxation);

    // for (unsigned int j = 0; j < po.pairs.size(); j++)
    //   if (po.pairs[j].first == MooseUtils::toUpper("-snes_linesearch_damping"))
    //     po.pairs[j].second = std::to_string(1.0);

    // _problem.computeResidualSys(
    //     pressure_system, *_pressure_sys.currentSolution(), *pressure_system.rhs);
    // pressure_residual = pressure_system.rhs->l2_norm();

    auto vw = pressure_system.current_local_solution->zero_clone();
    PetscVector<Number> * vw_petsc = dynamic_cast<PetscVector<Number> *>(vw.get());

    _problem.computeResidualSys(pressure_system, *vw, *pressure_system.rhs);

    std::cout << "Pressure RHS" << std::endl;
    pressure_system.rhs->print();

    // pressure_residual = pressure_system.rhs->l2_norm();

    // Moose::PetscSupport::petscSetOptions(_problem);

    std::cout << "old pressure" << std::endl;
    pressure_solution_old->print();

    _problem.solve(_pressure_sys_number);

    std::cout << "old pressure" << std::endl;
    pressure_solution_old->print();

    pressure_residual = _pressure_sys._initial_residual_after_preset_bcs;

    PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);

    std::cout << "Pressure matrix" << std::endl;
    pmat->print();

    _rc_uo->computeVelocity(_momentum_variable_relaxation);

    // PetscNonlinearSolver<Real> * solver =
    //     static_cast<PetscNonlinearSolver<Real> *>(pressure_system.nonlinear_solver.get());

    // PetscErrorCode ierr;
    // KSP linear_system;
    // ierr = SNESGetKSP(solver->snes(), &linear_system);
    // LIBMESH_CHKERR(ierr);

    // // Wpuld be cool to add asserts here
    // PetscVector<Number> * correction_petsc = dynamic_cast<PetscVector<Number>
    // *>(correction.get()); Vec v = correction_petsc->vec();

    // ierr = KSPGetSolution(linear_system, &v);
    // LIBMESH_CHKERR(ierr);

    // ierr = VecView(v, PETSC_VIEWER_STDOUT_SELF);

    // correction->print();

    PetscScalar relax = _pressure_variable_relaxation;

    std::cout << "new pressure" << std::endl;
    pressure_solution->print();
    std::cout << "old pressure" << std::endl;
    pressure_solution_old->print();

    pressure_solution->scale(relax);
    pressure_solution->add(1 - relax, *pressure_solution_old);

    // solution->print();
    pressure_solution->close();

    std::cout << "Pressure solution" << std::endl;
    pressure_solution->print();

    *pressure_solution_old = *pressure_solution;
    _pressure_sys.setSolution(*pressure_solution);

    std::cout << "old pressure" << std::endl;
    pressure_solution_old->print();

    // ierr = VecAYPX(solution->vec(), relax, vvv);

    _console << "Iteration " << iteration_counter << " Initial residual norms:\n"
             << " Momentum equation: " << COLOR_GREEN << momentum_residual << COLOR_DEFAULT << "\n"
             << " Pressure equation: " << COLOR_GREEN << pressure_residual << COLOR_DEFAULT
             << std::endl;
  }
}
