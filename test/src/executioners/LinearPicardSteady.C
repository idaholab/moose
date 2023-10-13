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

#include "libmesh/equation_systems.h"

registerMooseObject("MooseTestApp", LinearPicardSteady);

InputParameters
LinearPicardSteady::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addRequiredParam<NonlinearSystemName>("linear_sys_to_solve",
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
    _linear_sys_number(_problem.linearSysNum(getParam<NonlinearSystemName>("linear_sys_to_solve"))),
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
    _console << "Will solve al inear problem here" << std::endl;
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
