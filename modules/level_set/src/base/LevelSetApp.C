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

template <>
InputParameters
validParams<LevelSetApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.addClassDescription(
      "Application containing object necessary to solve the level set equation.");
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

static void
registerExecFlagsInner(Factory & factory)
{
  registerExecFlag(LevelSet::EXEC_ADAPT_MESH);
  registerExecFlag(LevelSet::EXEC_COMPUTE_MARKERS);
}

void
LevelSetApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"LevelSetApp"});
  Registry::registerActionsTo(af, {"LevelSetApp"});
  registerExecFlagsInner(f);
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
LevelSetApp::registerExecFlags(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
  registerExecFlagsInner(factory);
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
