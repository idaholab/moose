//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConditionalEnableControl.h"

InputParameters
ConditionalEnableControl::validParams()
{
  InputParameters params = Control::validParams();

  params.addParam<std::vector<std::string>>(
      "disable_objects", std::vector<std::string>(), "A list of object tags to disable.");
  params.addParam<std::vector<std::string>>(
      "enable_objects", std::vector<std::string>(), "A list of object tags to enable.");

  params.addParam<bool>("reverse_on_false",
                        true,
                        "When true, the disable/enable lists are set to opposite values when the "
                        "specified condition is false.");

  return params;
}

ConditionalEnableControl::ConditionalEnableControl(const InputParameters & parameters)
  : Control(parameters),
    _enable(getParam<std::vector<std::string>>("enable_objects")),
    _disable(getParam<std::vector<std::string>>("disable_objects")),
    _reverse_on_false(getParam<bool>("reverse_on_false"))
{
  // Error if enable and disable lists are both empty
  if (_enable.empty() && _disable.empty())
    mooseError(
        "Either or both of the 'enable_objects' and 'disable_objects' parameters must be set.");
}

void
ConditionalEnableControl::execute()
{
  // ENABLE
  for (MooseIndex(_enable) i = 0; i < _enable.size(); ++i)
    if (conditionMet(i))
      setControllableValueByName<bool>(_enable[i], std::string("enable"), true);
    else if (_reverse_on_false)
      setControllableValueByName<bool>(_enable[i], std::string("enable"), false);

  // DISABLE
  for (MooseIndex(_disable) i = 0; i < _disable.size(); ++i)
    if (conditionMet(i))
      setControllableValueByName<bool>(_disable[i], std::string("enable"), false);
    else if (_reverse_on_false)
      setControllableValueByName<bool>(_disable[i], std::string("enable"), true);
}
