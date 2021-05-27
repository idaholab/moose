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
#include "Transient.h"

registerMooseObject("MooseApp", IterationInfo);

InputParameters
IterationInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Report the time and iteration information for the simulation.");

  MultiMooseEnum items(
      "time timestep num_linear_iterations num_nonlinear_iterations num_fixed_point_iterations");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The iteration information to output, if nothing is provided everything will be output.");
  return params;
}

IterationInfo::IterationInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _items(getParam<MultiMooseEnum>("items")),
    _time_value(declareHelper<Real>("time", _dummy_real)),
    _time_step_value(declareHelper<unsigned int>("timestep", _dummy_unsigned_int)),
    _num_linear(declareHelper<unsigned int>("num_linear_iterations", _dummy_unsigned_int)),
    _num_nonlinear(declareHelper<unsigned int>("num_nonlinear_iterations", _dummy_unsigned_int)),
    _num_fixed_point(declareHelper<unsigned int>("num_fixed_point_iterations", _dummy_unsigned_int))
{
}

void
IterationInfo::execute()
{
  _time_value = _t;
  _time_step_value = _t_step;
  _num_nonlinear = _subproblem.nNonlinearIterations();
  _num_linear = _subproblem.nLinearIterations();
  _num_fixed_point = _app.getExecutioner()->fixedPointSolve().numFixedPointIts();
}
