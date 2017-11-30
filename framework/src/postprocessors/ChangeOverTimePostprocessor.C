/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ChangeOverTimePostprocessor.h"

template <>
InputParameters
validParams<ChangeOverTimePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  params.addParam<bool>("change_with_respect_to_initial",
                        false,
                        "Compute change with respect to initial value instead of previous value");
  params.addParam<bool>(
      "compute_relative_change", false, "Compute magnitude of relative change instead of change");
  params.addParam<bool>("take_absolute_value", false, "Option to take absolute value of change");

  params.addClassDescription("Computes the change or relative change in a post-processor value "
                             "over a timestep or the entire transient");

  return params;
}

ChangeOverTimePostprocessor::ChangeOverTimePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _change_with_respect_to_initial(getParam<bool>("change_with_respect_to_initial")),
    _compute_relative_change(getParam<bool>("compute_relative_change")),
    _take_absolute_value(getParam<bool>("take_absolute_value")),
    _pps_value(getPostprocessorValue("postprocessor")),
    _pps_value_old(getPostprocessorValueOld("postprocessor"))
{
  if (_change_with_respect_to_initial)
  {
    // ensure dependent post-processor is executed on initial
    const PostprocessorName & pp_name = getParam<PostprocessorName>("postprocessor");
    const UserObject & pp = _fe_problem.getUserObject<UserObject>(pp_name);
    const std::vector<ExecFlagType> & pp_exec_flags = pp.execFlags();
    if (std::find(pp_exec_flags.begin(), pp_exec_flags.end(), ExecFlagType::EXEC_INITIAL) ==
        pp_exec_flags.end())
      mooseError("When 'change_with_respect_to_initial' is specified to be true, 'execute_on' for "
                 "the dependent post-processor ('" +
                 pp_name + "') must include 'initial'");

    // ensure THIS post-processor is executed on initial
    if (std::find(_exec_flags.begin(), _exec_flags.end(), ExecFlagType::EXEC_INITIAL) ==
        _exec_flags.end())
      mooseError("When 'change_with_respect_to_initial' is specified to be true, 'execute_on' for "
                 "the ChangeOverTimePostprocessor ('" +
                 name() + "') must include 'initial'");
  }
}

void
ChangeOverTimePostprocessor::initialize()
{
}

void
ChangeOverTimePostprocessor::execute()
{
}

Real
ChangeOverTimePostprocessor::getValue()
{
  // copy initial value in case difference is measured against initial value
  if (_t_step == 0)
    _pps_value_initial = _pps_value;

  // determine value which change is measured against
  Real base_value;
  if (_change_with_respect_to_initial)
    base_value = _pps_value_initial;
  else
  {
    // Currently post-processors do not copy the values to old and older;
    // _pps_value_old will therefore always be zero for _t_step = 0.
    if (_t_step == 0)
      base_value = _pps_value;
    else
      base_value = _pps_value_old;
  }

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
