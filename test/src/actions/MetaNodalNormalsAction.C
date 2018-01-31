//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MetaNodalNormalsAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"

template <>
InputParameters
validParams<MetaNodalNormalsAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

MetaNodalNormalsAction::MetaNodalNormalsAction(const InputParameters & params) : Action(params) {}

void
MetaNodalNormalsAction::act()
{
  InputParameters object_params = _action_factory.getValidParams("AddNodalNormalsAction");
  object_params.set<ActionWarehouse *>("awh") = &_awh;

  // Create and Add First Variable Action
  auto action =
      _action_factory.create("AddNodalNormalsAction", "AddNodalNormalsAction", object_params);
  _awh.addActionBlock(action);
}
