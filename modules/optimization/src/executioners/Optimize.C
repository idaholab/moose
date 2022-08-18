//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Optimize.h"

registerMooseObject("OptimizationApp", Optimize);

InputParameters
Optimize::validParams()
{
  InputParameters params = Steady::validParams();
  params += OptimizeSolve::validParams();
  params.addClassDescription("Executioner for optimization problems.");
  return params;
}

Optimize::Optimize(const InputParameters & parameters) : Steady(parameters), _optim_solve(*this)
{
  _optim_solve.setInnerSolve(fixedPointSolve());
}

void
Optimize::execute()
{
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  _problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

  _problem.timestepSetup();

  _optim_solve.solve();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  _problem.execute(EXEC_TIMESTEP_END);
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  _problem.execMultiApps(EXEC_FINAL);
  _problem.finalizeMultiApps();
  _problem.execute(EXEC_FINAL);
  _time = _time_step;
  _problem.outputStep(EXEC_FINAL);
  _time = _system_time;

  postExecute();
}
