//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StartWebServerControlAction.h"
#include "ChainControl.h"
#include "FEProblemBase.h"
#include "WebServerControl.h"

registerMooseAction("MooseApp", StartWebServerControlAction, "start_webservercontrol");

InputParameters
StartWebServerControlAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Starts the web server(s) for the WebServerControl objects.");
  return params;
}

StartWebServerControlAction::StartWebServerControlAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
StartWebServerControlAction::act()
{
  for (auto & control_ptr : _problem->getControlWarehouse().getObjects())
    if (auto wsc_ptr = dynamic_cast<WebServerControl *>(control_ptr.get()))
      wsc_ptr->startServer({});
}
