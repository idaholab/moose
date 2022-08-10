//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "LevelSetApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "LevelSetTypes.h"

InputParameters
LevelSetApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.addClassDescription(
      "Application containing object necessary to solve the level set equation.");

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("LevelSetApp");

LevelSetApp::LevelSetApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());
  LevelSetApp::registerAll(_factory, _action_factory, _syntax);
}

void
LevelSetApp::registerApps()
{
  registerApp(LevelSetApp);
}

void
LevelSetApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"LevelSetApp"});
  Registry::registerActionsTo(af, {"LevelSetApp"});
}

void
LevelSetApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"LevelSetApp"});
}

void
LevelSetApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"LevelSetApp"});
}
void
LevelSetApp::registerExecFlags(Factory &)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

// Dynamic Library Entry Points - DO NOT MODIFY
extern "C" void
LevelSetApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  LevelSetApp::registerAll(f, af, s);
}
extern "C" void
LevelSetApp__registerApps()
{
  LevelSetApp::registerApps();
}
