//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusOptimizationSteady.h"

registerMooseObject("OptimizationApp", ExodusOptimizationSteady);

InputParameters
ExodusOptimizationSteady::validParams()
{
  // Get the base class parameters
  InputParameters params = Exodus::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  // Return the InputParameters
  return params;
}

ExodusOptimizationSteady::ExodusOptimizationSteady(const InputParameters & parameters)
  : Exodus(parameters), _steady_exec(dynamic_cast<Steady *>(_app.getExecutioner()))
{
  if (!_steady_exec)
    mooseError(
        "ExodusOptimizationSteady output can only be used with the SteadyAndAdjoint or Steady"
        "executioners to output a per-optimization iteration solution.");
}

void
ExodusOptimizationSteady::customizeFileOutput()
{
  if (_exodus_mesh_changed || _sequence)
    _file_num++;

  _exodus_num = _steady_exec->getIterationNumberOutput() + 1;

  if (_exodus_num == 1)
    _exodus_io_ptr->append(false);
  else
    _exodus_io_ptr->append(true);
}

Real
ExodusOptimizationSteady::getOutputTime()
{
  return _steady_exec->getIterationNumberOutput();
}
