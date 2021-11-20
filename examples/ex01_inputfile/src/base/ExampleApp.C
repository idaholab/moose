//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE Includes
#include "ExampleApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ExampleApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  return params;
}

ExampleApp::ExampleApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());
  ExampleApp::registerAll(_factory, _action_factory, _syntax);
}

void
ExampleApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ExampleApp"});
  Registry::registerActionsTo(af, {"ExampleApp"});
}

void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}
