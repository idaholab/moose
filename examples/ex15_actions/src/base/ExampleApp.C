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
#include "ActionFactory.h" // <- Actions are special (they have their own factory)
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
ExampleApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"ExampleApp"});
  Registry::registerActionsTo(af, {"ExampleApp"});

  /**
   * An Action is a little different than registering the other MOOSE
   * objects.  First, you need to register your Action like normal in its file with
   * the registerMooseAction macro. - e.g.:
   *
   *     registerMooseAction("ExampleApp", ConvectionDiffusionAction, "add_kernel");
   *
   * Then we need to tell the parser what new section name to look for and what
   * Action object to build when it finds it.  This is done directly on the syntax
   * with the registerActionSyntax method.
   *
   * The section name should be the "full path" of the parsed section but should NOT
   * contain a leading slash.  Wildcard characters can be used to replace a piece of the
   * path.
   */
  registerSyntax("ConvectionDiffusionAction", "ConvectionDiffusion");
}

void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}
