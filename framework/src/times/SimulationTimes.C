//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimulationTimes.h"

registerMooseObject("MooseApp", SimulationTimes);

InputParameters
SimulationTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Times simulated");

  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;
  // Times history of a simulation is already one-way
  params.set<bool>("auto_sort") = false;
  // TIMESTEP_BEGIN is as early and as often as we need
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  return params;
}

SimulationTimes::SimulationTimes(const InputParameters & parameters) : Times(parameters) {}

void
SimulationTimes::initialize()
{
  // Initialize is by default what is called by execute()
  _times.push_back(_fe_problem.time());
  // if this is performed multiple times (fixed point iterations)
  // it will be caught by our logic to make the vector hold unique times in finalize()
}
