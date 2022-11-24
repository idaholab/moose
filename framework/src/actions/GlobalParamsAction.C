//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalParamsAction.h"

#include "MooseApp.h"
#include "InputParameterWarehouse.h"

registerMooseAction("MooseApp", GlobalParamsAction, "set_global_params");

InputParameters
GlobalParamsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action used to aid in the application of parameters defined in the "
                             "GlobalParams input block.");
  std::vector<std::string> blocks(1, "");

  /* GlobalParams should not have children or other standard public Action attributes */
  params.addPrivateParam<std::vector<std::string>>("active", blocks);
  params.addPrivateParam<std::vector<std::string>>("inactive", blocks);
  return params;
}

GlobalParamsAction::GlobalParamsAction(const InputParameters & params) : Action(params) {}

void
GlobalParamsAction::act()
{
}

void
GlobalParamsAction::remove(const std::string & name)
{
  parameters().remove(name);
}

InputParameters &
GlobalParamsAction::parameters()
{
  return const_cast<InputParameters &>(_pars);
}
