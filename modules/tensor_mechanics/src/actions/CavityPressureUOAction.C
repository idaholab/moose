//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressureUOAction.h"

#include "CavityPressureUserObject.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseAction("TensorMechanicsApp", CavityPressureUOAction, "add_user_object");

InputParameters
CavityPressureUOAction::validParams()
{
  InputParameters params = Action::validParams();
  params += CavityPressureUserObject::validParams();

  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = EXEC_LINEAR;
  params.addParam<ExecFlagEnum>("execute_on", exec_enum, exec_enum.getDocString());
  params.addClassDescription("Action to add user objects for cavity pressure");
  return params;
}

CavityPressureUOAction::CavityPressureUOAction(const InputParameters & params) : Action(params) {}

void
CavityPressureUOAction::act()
{
  InputParameters params = _factory.getValidParams("CavityPressureUserObject");

  params.applyParameters(parameters());

  _problem->addUserObject("CavityPressureUserObject", _name + "UserObject", params);
}
