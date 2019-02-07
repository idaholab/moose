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
    _final_timer(registerTimedSection("final", 1))
{
  _problem.getNonlinearSystemBase().setDecomposition(_splitting);

  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);
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
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
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

  // use timeStep to control the mesh adaptation and output
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR
    _problem.timestepSetup();

    _last_solve_converged = _picard_solve.solve();

    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge\n";
      break;
    }

    _problem.computeIndicators();
    _problem.computeMarkers();

    _problem.outputStep(EXEC_TIMESTEP_END);

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
    {
      _problem.adaptMesh();
    }

    _time_step++;
  }
#endif

  {
    TIME_SECTION(_final_timer)
    _problem.execute(EXEC_FINAL);
    _problem.outputStep(EXEC_FINAL);
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
