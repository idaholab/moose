//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Steady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseApp", Steady);

InputParameters
Steady::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for steady-state simulations.");
  params += FEProblemSolve::validParams();
  params.addParam<Real>("time", 0.0, "System time");
  return params;
}

Steady::Steady(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _system_time(getParam<Real>("time")),
    _time_step(_problem.timeStep()),
    _time(_problem.time())
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  _time = _system_time;
}

void
Steady::init()
{
  checkIntegrity();
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();
}

void
Steady::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _time_step = 0;
  _time = _time_step;
  _problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  _problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR
    _problem.timestepSetup();

    _last_solve_converged = _fixed_point_solve->solve();

    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge" << std::endl;
      break;
    }

    _problem.computeIndicators();
    _problem.computeMarkers();

    // need to keep _time in sync with _time_step to get correct output
    _time = _time_step;
    _problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
    {
      _problem.adaptMesh();
    }

    _time_step++;
  }
#endif

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiApps(EXEC_FINAL);
    _problem.finalizeMultiApps();
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}

void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (_problem.getNonlinearSystemBase().containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation");
}
