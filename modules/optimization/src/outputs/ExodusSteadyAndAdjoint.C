//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusSteadyAndAdjoint.h"

registerMooseObject("OptimizationApp", ExodusSteadyAndAdjoint);

InputParameters
ExodusSteadyAndAdjoint::validParams()
{
  // Get the base class parameters
  InputParameters params = Exodus::validParams();

  // Return the InputParameters
  return params;
}

ExodusSteadyAndAdjoint::ExodusSteadyAndAdjoint(const InputParameters & parameters)
  : Exodus(parameters),
    _steady_and_adjoint_exec(dynamic_cast<SteadyAndAdjoint *>(_app.getExecutioner()))
{
  if (!_steady_and_adjoint_exec)
    mooseError("ExodusSteadyAndAdjoint output can only be used with the SteadyAndAdjoint "
               "executioner to output a per-optimization iteration solution.");
}

void
ExodusSteadyAndAdjoint::incrementFileCounter()
{
  _file_num = _steady_and_adjoint_exec->getIterationNumberOutput();
}

Real
ExodusSteadyAndAdjoint::getTimeStepForOutput()
{
  return _steady_and_adjoint_exec ? _steady_and_adjoint_exec->getIterationNumberOutput() : time();
}
