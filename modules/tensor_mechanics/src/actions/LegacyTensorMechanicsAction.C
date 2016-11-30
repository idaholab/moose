/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
