//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Receiver.h"

template <>
InputParameters
validParams<Receiver>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<Real>("default", "The default value");
  params.addParam<bool>(
      "initialize_old", true, "Initialize the old postprocessor value with the default value");
  return params;
}

Receiver::Receiver(const InputParameters & params)
  : GeneralPostprocessor(params),
    _initialize_old(getParam<bool>("initialize_old")),
    _my_value(getPostprocessorValueByName(name()))
{
}

Real
Receiver::getValue()
{
  // Return the stored value (references stored value in getPostprocessorData)
  return _my_value;
}

void
Receiver::initialSetup()
{
  if (isParamValid("default"))
  {
    Real value = getParam<Real>("default");
    _fe_problem.getPostprocessorValue(_pp_name) = value;
    if (_initialize_old)
    {
      _fe_problem.getPostprocessorValueOld(_pp_name) = value;
      _fe_problem.getPostprocessorValueOlder(_pp_name) = value;
    }
  }
}
