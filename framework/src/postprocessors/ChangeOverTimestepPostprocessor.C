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

#include "ChangeOverTimestepPostprocessor.h"

template <>
InputParameters
validParams<ChangeOverTimestepPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  params.addParam<bool>(
      "compute_relative_change", false, "Compute magnitude of relative change instead of change");
  return params;
}

ChangeOverTimestepPostprocessor::ChangeOverTimestepPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _compute_relative_change(getParam<bool>("compute_relative_change")),
    _pps_value(getPostprocessorValue("postprocessor")),
    _pps_value_old(getPostprocessorValueOld("postprocessor"))
{
}

void
ChangeOverTimestepPostprocessor::initialize()
{
}

void
ChangeOverTimestepPostprocessor::execute()
{
}

Real
ChangeOverTimestepPostprocessor::getValue()
{
  if (_compute_relative_change)
    return std::fabs((std::fabs(_pps_value) - std::fabs(_pps_value_old)) *
                     std::pow(std::fabs(_pps_value), -1));
  else
    return _pps_value - _pps_value_old;
}
