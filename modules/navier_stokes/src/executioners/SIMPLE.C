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

  unsigned int iteration_counter = 0;
  _problem.computeResidualSys(
      momentum_system, *_momentum_sys.currentSolution(), *momentum_system.rhs);
  Real momentum_residual = momentum_system.rhs->l2_norm();
  Real pressure_residual = 1.0;
  while (iteration_counter < _num_iterations && (momentum_residual > _momentum_absolute_tolerance &&
                                                 pressure_residual > _pressure_absolute_tolerance))
  {
    iteration_counter++;
    if (iteration_counter)
    {
      _problem.computeResidualSys(
          momentum_system, *_momentum_sys.currentSolution(), *momentum_system.rhs);
      momentum_residual = momentum_system.rhs->l2_norm();
    }

    for (unsigned int j = 0; j < po.pairs.size(); j++)
      if (po.pairs[j].first == MooseUtils::toUpper("-snes_linesearch_damping"))
        po.pairs[j].second = std::to_string(_momentum_variable_relaxation);
    Moose::PetscSupport::petscSetOptions(_problem);

    _problem.solve(_momentum_sys_number);

    _rc_uo->execute();

    for (unsigned int j = 0; j < po.pairs.size(); j++)
      if (po.pairs[j].first == MooseUtils::toUpper("-snes_linesearch_damping"))
        po.pairs[j].second = std::to_string(_pressure_variable_relaxation);

    _problem.computeResidualSys(
        pressure_system, *_pressure_sys.currentSolution(), *pressure_system.rhs);
    pressure_residual = pressure_system.rhs->l2_norm();

    Moose::PetscSupport::petscSetOptions(_problem);
    _problem.solve(_pressure_sys_number);

    PetscMatrix<Number> * pmat = dynamic_cast<PetscMatrix<Number> *>(pressure_system.matrix);
    PetscVector<Number> * solution =
        dynamic_cast<PetscVector<Number> *>(pressure_system.current_local_solution.get());

    std::cout << "Pressure matrix" << std::endl;
    pmat->print();

    std::cout << "Pressure solution" << std::endl;
    solution->print();

    _rc_uo->computeVelocity();

    _console << "Iteration " << iteration_counter << " Initial residual norms:\n"
             << " Momentum equation: " << COLOR_GREEN << momentum_residual << COLOR_DEFAULT << "\n"
             << " Pressure equation: " << COLOR_GREEN << pressure_residual << COLOR_DEFAULT
             << std::endl;
  }
}
