//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODTransient.h"

// MOOSE includes
#include "Factory.h"
#include "SubProblem.h"
#include "TimeStepper.h"
#include "MooseApp.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "Control.h"
#include "TimePeriod.h"
#include "MooseMesh.h"
#include "AllLocalDofIndicesThread.h"
#include "TimeIntegrator.h"
#include "Console.h"

#include "StochasticToolsTypes.h"

registerMooseObject("StochasticToolsApp", PODTransient);

defineLegacyParams(PODTransient);

InputParameters
PODTransient::validParams()
{
  InputParameters params = Transient::validParams();
  return params;
}

PODTransient::PODTransient(const InputParameters & parameters) : Transient(parameters) {}

void
PODTransient::execute()
{
  preExecute();

  // Start time loop...
  while (keepGoing())
  {
    incrementStepOrReject();
    preStep();
    computeDT();
    takeStep();
    endStep();
    postStep();
  }

  if (lastSolveConverged())
  {
    _t_step++;

    /*
     * Call the multi-app executioners endStep and
     * postStep methods when doing Picard or when not automatically advancing sub-applications for
     * some other reason. We do not perform these calls for loose-coupling/auto-advancement
     * problems because Transient::endStep and Transient::postStep get called from
     * TransientMultiApp::solveStep in that case.
     */
    if (!_picard_solve.autoAdvance())
    {
      _problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN, /*recurse_through_multiapp_levels=*/true);
      _problem.finishMultiAppStep(EXEC_TIMESTEP_END, /*recurse_through_multiapp_levels=*/true);
    }
  }

  if (!_app.halfTransient())
  {
    {
      TIME_SECTION(_post_snapshot_timer);
      _problem.execMultiApps(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
      _problem.finalizeMultiApps();
      _problem.execute(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
      _problem.outputStep(StochasticTools::EXEC_POST_SNAPSHOT_GEN);
    }

    {
      TIME_SECTION(_final_timer);
      _problem.execMultiApps(EXEC_FINAL);
      _problem.finalizeMultiApps();
      _problem.execute(EXEC_FINAL);
      _problem.outputStep(EXEC_FINAL);
    }
  }

  // This method is to finalize anything else we want to do on the problem side.
  _problem.postExecute();

  // This method can be overridden for user defined activities in the Executioner.
  postExecute();
}
