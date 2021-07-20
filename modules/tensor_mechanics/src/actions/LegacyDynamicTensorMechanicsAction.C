//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LegacyDynamicTensorMechanicsAction.h"

registerMooseAction("TensorMechanicsApp",
                    LegacyDynamicTensorMechanicsAction,
                    "setup_mesh_complete");

registerMooseAction("TensorMechanicsApp",
                    LegacyDynamicTensorMechanicsAction,
                    "validate_coordinate_systems");

registerMooseAction("TensorMechanicsApp", LegacyDynamicTensorMechanicsAction, "add_kernel");

InputParameters
LegacyDynamicTensorMechanicsAction::validParams()
{
  InputParameters params = DynamicTensorMechanicsAction::validParams();
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

LegacyDynamicTensorMechanicsAction::LegacyDynamicTensorMechanicsAction(
    const InputParameters & params)
  : DynamicTensorMechanicsAction(params)
{
}

void
LegacyDynamicTensorMechanicsAction::act()
{
  if (_current_task == "add_kernel" || _current_task == "validate_coordinate_systems")
    // note that we do not call DynamicTensorMechanicsAction::act() here, because the old
    // behavior is not to add inertia kernels
    TensorMechanicsAction::act();
}
