//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IterationInfo.h"

#include "SubProblem.h"

registerMooseObject("MooseApp", IterationInfo);

InputParameters
IterationInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  return params;
}

IterationInfo::IterationInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _time_value(declareValue<Real>("time")),
    _time_step_value(declareValue<unsigned int>("timestep")),
    _num_linear(declareValue<unsigned int>("num_linear_iterations")),
    _num_nonlinear(declareValue<unsigned int>("num_nonlinear_iterations"))
{
}

void
IterationInfo::execute()
{
  _time_value = _t;
  _time_step_value = _t_step;
  _num_nonlinear = _subproblem.nNonlinearIterations();
  _num_linear = _subproblem.nLinearIterations();
}
