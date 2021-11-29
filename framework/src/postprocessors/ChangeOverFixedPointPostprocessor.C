//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// moose includes
#include "ChangeOverFixedPointPostprocessor.h"
#include "Transient.h"

registerMooseObject("MooseApp", ChangeOverFixedPointPostprocessor);

InputParameters
ChangeOverFixedPointPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  params.addParam<bool>("change_with_respect_to_initial",
                        false,
                        "Compute change with respect to value at the beginning of the FixedPoint "
                        "iterations instead of previous value");
  params.addParam<bool>(
      "compute_relative_change", false, "Compute magnitude of relative change instead of change");
  params.addParam<bool>("take_absolute_value", false, "Option to take absolute value of change");

  params.addClassDescription("Computes the change or relative change in a post-processor value "
                             "over a single or multiple fixed point iterations");

  return params;
}

ChangeOverFixedPointPostprocessor::ChangeOverFixedPointPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _change_with_respect_to_initial(getParam<bool>("change_with_respect_to_initial")),
    _compute_relative_change(getParam<bool>("compute_relative_change")),
    _take_absolute_value(getParam<bool>("take_absolute_value")),
    _pps_value(getPostprocessorValue("postprocessor")),
    _pps_value_old(0),
    _pps_value_initial(declareRestartableData<Real>("pps_value_initial")),
    _t_step_old(-1)
{
  if (_change_with_respect_to_initial)
  {
    // ensure dependent post-processor is executed on initial
    const PostprocessorName & pp_name = getParam<PostprocessorName>("postprocessor");
    const UserObject & pp = _fe_problem.getUserObject<UserObject>(pp_name);
    if (!pp.getExecuteOnEnum().contains(EXEC_INITIAL))
      mooseError("When 'change_with_respect_to_initial' is specified to be true, 'execute_on' for "
                 "the dependent post-processor ('" +
                 pp_name + "') must include 'initial'");

    // ensure THIS post-processor is executed on initial
    if (!_execute_enum.contains(EXEC_INITIAL))
      mooseError("When 'change_with_respect_to_initial' is specified to be true, 'execute_on' for "
                 "the ChangeOverFixedPointPostprocessor ('" +
                 name() + "') must include 'initial'");
  }
}

void
ChangeOverFixedPointPostprocessor::initialize()
{
}

void
ChangeOverFixedPointPostprocessor::execute()
{
}

Real
ChangeOverFixedPointPostprocessor::getValue()
{
  // detect the beginning of a new FixedPoint iteration process
  // it can either a new time step or a failed time step
  bool new_time_step = false;
  if (_app.getExecutioner()->fixedPointSolve().numFixedPointIts() == 1)
  {
    // new time step
    if (_t_step != _t_step_old)
    {
      new_time_step = true;
      _t_step_old = _t_step;

      // copy initial value in case difference is measured against initial value
      // or for a reset if the time step fails
      _pps_value_initial = _pps_value;
    }
    // failed time step
    else
      _pps_value_old = _pps_value_initial;
  }

  // determine value which change is measured against
  Real base_value;
  if (_change_with_respect_to_initial)
    base_value = _pps_value_initial;
  else
  {
    // for a new time step, initial value is the reference
    if (new_time_step)
      base_value = _pps_value_initial;
    else
      base_value = _pps_value_old;
  }

  // Update previous value
  _pps_value_old = _pps_value;

  // compute change in value
  Real change;
  if (_compute_relative_change)
    change = (_pps_value - base_value) / base_value;
  else
    change = _pps_value - base_value;

  if (_take_absolute_value)
    return std::fabs(change);
  else
    return change;
}
