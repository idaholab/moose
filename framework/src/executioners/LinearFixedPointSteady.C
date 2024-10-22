//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearFixedPointSteady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"
#include "libmesh/linear_implicit_system.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseApp", LinearFixedPointSteady);

InputParameters
LinearFixedPointSteady::validParams()
{
  InputParameters params = LinearFixedPointSolve::validParams();
  params += Executioner::validParams();
  params.addParam<Real>("time", 0.0, "System time");

  return params;
}

LinearFixedPointSteady::LinearFixedPointSteady(const InputParameters & parameters)
  : Executioner(parameters),
    _solve(*this),
    _system_time(getParam<Real>("time")),
    _time_step(_fe_problem.timeStep()),
    _time(_fe_problem.time())
{
  _fixed_point_solve->setInnerSolve(_solve);
  _time = _system_time;
}

void
LinearFixedPointSteady::init()
{
  _fe_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _fe_problem.initialSetup();
}

void
LinearFixedPointSteady::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _time_step = 0;
  _time = _time_step;
  _fe_problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  _fe_problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _fe_problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR
    _fe_problem.timestepSetup();

    _last_solve_converged = _fixed_point_solve->solve();

    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge" << std::endl;
      break;
    }

    _fe_problem.computeIndicators();
    _fe_problem.computeMarkers();

    // need to keep _time in sync with _time_step to get correct output
    _time = _time_step;
    _fe_problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
      _fe_problem.adaptMesh();

    _time_step++;
  }
#endif

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _fe_problem.execMultiApps(EXEC_FINAL);
    _fe_problem.finalizeMultiApps();
    _fe_problem.postExecute();
    _fe_problem.execute(EXEC_FINAL);
    _time = _time_step;
    _fe_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}
