//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearPicardSteady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"
#include "ComputeLinearFVElementalThread.h"
#include "ComputeLinearFVFaceThread.h"
#include "libmesh/linear_implicit_system.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseTestApp", LinearPicardSteady);

InputParameters
LinearPicardSteady::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addRequiredParam<LinearSystemName>("linear_sys_to_solve",
                                            "The first nonlinear system to solve");
  params.addRangeCheckedParam<unsigned int>("number_of_iterations",
                                            1,
                                            "number_of_iterations>0",
                                            "The number of iterations between the two systems.");
  return params;
}

LinearPicardSteady::LinearPicardSteady(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _linear_sys_number(_problem.linearSysNum(getParam<LinearSystemName>("linear_sys_to_solve"))),
    _number_of_iterations(getParam<unsigned int>("number_of_iterations"))
{
  _time = 0;
}

void
LinearPicardSteady::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _problem.initialSetup();
}

void
LinearPicardSteady::execute()
{
  if (_app.isRecovering())
    return;

  _time_step = 0;
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

  _problem.timestepSetup();

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  for (unsigned int i = 0; i < _number_of_iterations; i++)
  {
    newSolve();
  }

  // need to keep _time in sync with _time_step to get correct output
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

  postExecute();
}

void
LinearPicardSteady::originalSolve()
{
  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  ElemInfoRange elem_info_range(_problem.mesh().ownedElemInfoBegin(),
                                _problem.mesh().ownedElemInfoEnd());

  using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  FaceInfoRange face_info_range(_problem.mesh().ownedFaceInfoBegin(),
                                _problem.mesh().ownedFaceInfoEnd());

  ComputeLinearFVElementalThread elem_thread(
      _problem, 0, Moose::FV::LinearFVComputationMode::FullSystem, {});
  Threads::parallel_reduce(elem_info_range, elem_thread);

  ComputeLinearFVFaceThread face_thread(
      _problem, 0, Moose::FV::LinearFVComputationMode::FullSystem, {});
  Threads::parallel_reduce(face_info_range, face_thread);

  auto & sys = _problem.getLinearSystem(0);
  LinearImplicitSystem & lisystem = libMesh::cast_ref<LinearImplicitSystem &>(sys.system());

  lisystem.matrix->close();
  lisystem.rhs->close();

  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*lisystem.get_linear_solver());

  KSPSetNormType(linear_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // LinearPicardSolverConfiguration solver_config;
  // solver_config.real_valued_data["abs_tol"] = 1e-7;
  // linear_solver.set_solver_configuration(solver_config);

  linear_solver.solve(
      *lisystem.matrix, *lisystem.matrix, *lisystem.solution, *lisystem.rhs, 1e-10, 500);
  lisystem.update();

  lisystem.matrix->print();
  lisystem.rhs->print();
  lisystem.solution->print();
}

void
LinearPicardSteady::newSolve()
{
  auto & sys = _problem.getLinearSystem(0);
  LinearImplicitSystem & lisystem = libMesh::cast_ref<LinearImplicitSystem &>(sys.system());

  sys.solve();

  lisystem.matrix->print();
  lisystem.rhs->print();
  lisystem.solution->print();
}
