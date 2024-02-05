//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonSolidMechanicsAction.h"
#include "QuasiStaticSolidMechanicsPhysics.h"
#include "ActionWarehouse.h"

registerMooseAction("SolidMechanicsApp", CommonSolidMechanicsAction, "meta_action");

InputParameters
CommonSolidMechanicsAction::validParams()
{
  InputParameters params = QuasiStaticSolidMechanicsPhysicsBase::validParams();
  params.addClassDescription("Store common solid mechanics parameters");
  return params;
}

CommonSolidMechanicsAction::CommonSolidMechanicsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CommonSolidMechanicsAction::act()
{
  // check if sub-blocks block are found which will use the common parameters
  auto action = _awh.getActions<QuasiStaticSolidMechanicsPhysicsBase>();
  if (action.size() == 0)
    mooseWarning("Common parameters are supplied, but not used in ", parameters().blockLocation());
}
