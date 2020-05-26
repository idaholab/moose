//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonTensorMechanicsAction.h"
#include "TensorMechanicsAction.h"
#include "ActionWarehouse.h"

registerMooseAction("TensorMechanicsApp", CommonTensorMechanicsAction, "meta_action");

InputParameters
CommonTensorMechanicsAction::validParams()
{
  InputParameters params = TensorMechanicsActionBase::validParams();
  params.addClassDescription("Store common tensor mechanics parameters");
  return params;
}

CommonTensorMechanicsAction::CommonTensorMechanicsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CommonTensorMechanicsAction::act()
{
  // check if sub-blocks block are found which will use the common parameters
  auto action = _awh.getActions<TensorMechanicsActionBase>();
  if (action.size() == 0)
    mooseWarning("Common parameters are supplied, but not used in ", parameters().blockLocation());
}
