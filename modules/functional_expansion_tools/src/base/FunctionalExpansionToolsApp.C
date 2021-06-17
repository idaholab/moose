//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionalExpansionToolsApp.h"

#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FunctionalExpansionToolsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("FunctionalExpansionToolsApp");

FunctionalExpansionToolsApp::FunctionalExpansionToolsApp(InputParameters parameters)
  : MooseApp(parameters)
{
  FunctionalExpansionToolsApp::registerAll(_factory, _action_factory, _syntax);
}

FunctionalExpansionToolsApp::~FunctionalExpansionToolsApp() {}

void
FunctionalExpansionToolsApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
}

void
FunctionalExpansionToolsApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"FunctionalExpansionToolsApp"});
  Registry::registerActionsTo(af, {"FunctionalExpansionToolsApp"});
}

void
FunctionalExpansionToolsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"FunctionalExpansionToolsApp"});
}

void
FunctionalExpansionToolsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"FunctionalExpansionToolsApp"});
  /* Uncomment Syntax parameters and register your new objects here! */
}

extern "C" void
FunctionalExpansionToolsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FunctionalExpansionToolsApp::registerAll(f, af, s);
}
extern "C" void
FunctionalExpansionToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
}
