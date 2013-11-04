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

#include "Steady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "libmesh/equation_systems.h"

template<>
InputParameters validParams<Steady>()
{
  return validParams<Executioner>();
}


Steady::Steady(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*getParam<FEProblem *>("_fe_problem")),
    _time_step(_problem.timeStep()),
    _time(_problem.time())
{
  if (!_restart_file_base.empty())
    _problem.setRestartFile(_restart_file_base);

  {
    std::string ti_str = "SteadyState";
    InputParameters params = _app.getFactory().getValidParams(ti_str);
    _problem.addTimeIntegrator(ti_str, "ti", params);
  }
}

Steady::~Steady()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

Problem &
Steady::problem()
{
  return _problem;
}

void
Steady::init()
{
  checkIntegrity();

  _problem.initialSetup();
  Moose::setup_perf_log.push("Output Initial Condition","Setup");
  if (_output_initial)
  {
    _problem.output();
    _problem.outputPostprocessors();
    _problem.outputRestart();
  }
  Moose::setup_perf_log.pop("Output Initial Condition","Setup");
}

void
Steady::execute()
{
  Moose::out << "Time: " << _time_step << '\n';
  preExecute();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;
  _time = _time_step;                 // need to keep _time in sync with _time_step to get correct output

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for(unsigned int r_step=0; r_step<=steps; r_step++)
  {
#endif //LIBMESH_ENABLE_AMR
    _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::PRE_AUX);
    preSolve();
    _problem.updateMaterials();
    _problem.timestepSetup();
    _problem.computeUserObjects(EXEC_TIMESTEP_BEGIN, UserObjectWarehouse::POST_AUX);
    _problem.solve();
    postSolve();

    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
    _problem.onTimestepEnd();

    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
    _problem.computeIndicatorsAndMarkers();

    _problem.output();
    _problem.outputPostprocessors();
    _problem.outputRestart();

#ifdef LIBMESH_ENABLE_AMR
    if(r_step != steps)
    {
      _problem.adaptMesh();
    }

    _time_step++;
    _time = _time_step;                 // need to keep _time in sync with _time_step to get correct output
  }
#endif

  postExecute();
}

void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (_problem.getNonlinearSystem().containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation");
}
