//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonCohesiveZoneAction.h"
#include "CohesiveZoneActionBase.h"
#include "ActionWarehouse.h"

registerMooseAction("TensorMechanicsApp", CommonCohesiveZoneAction, "meta_action");

InputParameters
CommonCohesiveZoneAction::validParams()
{
  InputParameters params = CohesiveZoneActionBase::validParams();
  params.addClassDescription("Store common cohesive zone paramters");
  return params;
}

CommonCohesiveZoneAction::CommonCohesiveZoneAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CommonCohesiveZoneAction::act()
{
  // check if sub-blocks block are found which will use the common parameters
  auto action = _awh.getActions<CohesiveZoneActionBase>();
  if (action.size() == 0)
    mooseWarning("Common parameters are supplied, but not used in ", parameters().blockLocation());
}
