/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "Steady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/equation_systems.h"

template <>
InputParameters
validParams<Steady>()
{
  return validParams<Executioner>();
}

Steady::Steady(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time())
{
  _problem.getNonlinearSystemBase().setDecomposition(_splitting);

  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);

  {
    std::string ti_str = "SteadyState";
    InputParameters params = _app.getFactory().getValidParams(ti_str);
    _problem.addTimeIntegrator(ti_str, "ti", params);
  }
}

void
Steady::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  checkIntegrity();
  _problem.initialSetup();

  _problem.outputStep(EXEC_INITIAL);
}

void
Steady::execute()
{
  if (_app.isRecovering())
    return;

  preExecute();

  _problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;
  _time = _time_step; // need to keep _time in sync with _time_step to get correct output

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR
    preSolve();
    _problem.timestepSetup();
    _problem.execute(EXEC_TIMESTEP_BEGIN);
    _problem.outputStep(EXEC_TIMESTEP_BEGIN);

    // Update warehouse active objects
    _problem.updateActiveObjects();

    _problem.solve();
    postSolve();

    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge\n";
      break;
    }

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);

    _problem.computeIndicators();
    _problem.computeMarkers();

    _problem.outputStep(EXEC_TIMESTEP_END);

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
    {
      _problem.adaptMesh();
    }

    _time_step++;
    _time = _time_step; // need to keep _time in sync with _time_step to get correct output
  }
#endif

  postExecute();
}

void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (_problem.getNonlinearSystemBase().containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation");
}
