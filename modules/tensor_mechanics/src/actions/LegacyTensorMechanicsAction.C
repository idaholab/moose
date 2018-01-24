//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LegacyTensorMechanicsAction.h"

template <>
InputParameters
validParams<LegacyTensorMechanicsAction>()
{
  return validParams<TensorMechanicsAction>();
}

LegacyTensorMechanicsAction::LegacyTensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params)
{
}

void
LegacyTensorMechanicsAction::act()
{
  if (_current_task == "add_kernel" || _current_task == "validate_coordinate_systems")
    TensorMechanicsAction::act();
}
