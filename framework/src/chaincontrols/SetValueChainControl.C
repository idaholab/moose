//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetValueChainControl.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", SetRealValueChainControl);
registerMooseObject("MooseApp", SetBoolValueChainControl);

template <typename T>
InputParameters
SetValueChainControlTempl<T>::validParams()
{
  InputParameters params = ChainControl::validParams();
  params.addRequiredParam<std::string>("parameter", "Parameter(s) to control");
  params.addRequiredParam<std::string>("value",
                                       "Control data to use for the value of the parameter(s)");
  const std::string type_str = MooseUtils::prettyCppType<T>();
  params.addClassDescription("Sets parameter(s) of type '" + type_str +
                             "' to a control data value of type '" + type_str + "'.");
  return params;
}

template <typename T>
SetValueChainControlTempl<T>::SetValueChainControlTempl(const InputParameters & parameters)
  : ChainControl(parameters), _value(getChainControlData<T>("value"))
{
}

template <typename T>
void
SetValueChainControlTempl<T>::execute()
{
  setControllableValue<T>("parameter", _value);
}
