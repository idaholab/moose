//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlexibleExecutioner.h"

registerMooseObject("MooseTestApp", FlexibleExecutioner);

InputParameters
FlexibleExecutioner::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addParam<std::vector<std::string>>("solve_object_ordering", "Ordering of solve objects");
  return params;
}

FlexibleExecutioner::FlexibleExecutioner(const InputParameters & parameters)
  : Executioner(parameters)
{
}

void
FlexibleExecutioner::execute()
{
  if (!_head_solve_object)
    mooseError("Executioner has not been set up for executioner with 'solver_object_ordering'");

  // set initial time and do an initial output
  _time_step = 0;
  _time = 0;
  _fe_problem.outputStep(EXEC_INITIAL);

  preExecute();

  _last_solve_converged = _head_solve_object->solve();

  {
    _fe_problem.onTimestepEnd();
    _fe_problem.execute(EXEC_TIMESTEP_END);

    ++_time_step;
    Real t = _time;
    _time = _time_step;
    _fe_problem.outputStep(EXEC_TIMESTEP_END);
    _time = t;
  }

  {
    _fe_problem.execMultiApps(EXEC_FINAL);
    _fe_problem.finalizeMultiApps();
    _fe_problem.execute(EXEC_FINAL);
    // FIXME: output time should be managed by solve objects through output system
    ++_time_step;
    Real t = _time;
    _time = _time_step;
    _fe_problem.outputStep(EXEC_FINAL);
    _time = t;
  }

  postExecute();
};
