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
ExodusOptimizationSteady::incrementFileCounter()
{
  _file_num = _steady_exec->getIterationNumberOutput();
}

Real
ExodusOptimizationSteady::getOutputTime()
{
  return _steady_exec->getIterationNumberOutput();
}
