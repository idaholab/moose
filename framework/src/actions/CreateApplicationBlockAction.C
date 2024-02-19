//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateApplicationBlockAction.h"
#include "MooseApp.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", CreateApplicationBlockAction, "create_application_block");

InputParameters
CreateApplicationBlockAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::string>(
      "type", "", "The name of the application that should run this input file.");

  params.addClassDescription("Adds application and application related parameters.");

  return params;
}

CreateApplicationBlockAction::CreateApplicationBlockAction(const InputParameters & parameters)
  : Action(parameters), _type(getParam<std::string>("type"))
{
}

void
CreateApplicationBlockAction::act()
{
}
