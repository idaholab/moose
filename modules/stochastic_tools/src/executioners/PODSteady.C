//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODSteady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "StochasticToolsApp.h"
#include "StochasticToolsTypes.h"
#include "NonlinearSystem.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseApp", PODSteady);

defineLegacyParams(PODSteady);

InputParameters
PODSteady::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for steady-state simulations.");
  params.addParam<Real>("time", 0.0, "System time");
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on", true);
  exec.addAvailableFlags(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
  return params;
}

PODSteady::PODSteady(const InputParameters & parameters) : Steady(parameters) {}

void
PODSteady::execute()
{
  if (_app.isRecovering())
    return;

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

    for (MooseIndex(_num_grid_steps) grid_step = 0; grid_step <= _num_grid_steps; ++grid_step)
    {
      _last_solve_converged = _picard_solve.solve();

      if (!lastSolveConverged())
      {
        _console << "Aborting as solve did not converge\n";
        break;
      }

      if (grid_step != _num_grid_steps)
        _problem.uniformRefine();
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
    TIME_SECTION(_post_snapshot_timer)
    _problem.execMultiApps(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
    _problem.execute(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
    _time = _time_step;
    _problem.outputStep(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
    _time = _system_time;
  }

  {
    TIME_SECTION(_final_timer)
    _problem.execMultiApps(EXEC_FINAL);
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}
