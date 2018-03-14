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

  Moose::registerObjects(_factory);
  LevelSetApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LevelSetApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  LevelSetApp::registerExecFlags(_factory);
}

void
LevelSetApp::registerApps()
{
  registerApp(LevelSetApp);
}

void
LevelSetApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"LevelSetApp"});
}

void
LevelSetApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"LevelSetApp"});
}

void
LevelSetApp::registerExecFlags(Factory & factory)
{
  registerExecFlag(LevelSet::EXEC_ADAPT_MESH);
  registerExecFlag(LevelSet::EXEC_COMPUTE_MARKERS);
}

// Dynamic Library Entry Points - DO NOT MODIFY
extern "C" void
LevelSetApp__registerApps()
{
  LevelSetApp::registerApps();
}

extern "C" void
LevelSetApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  LevelSetApp::associateSyntax(syntax, action_factory);
}

extern "C" void
LevelSetApp__registerExecFlags(Factory & factory)
{
  LevelSetApp::registerExecFlags(factory);
}

extern "C" void
LevelSetApp__registerObjects(Factory & factory)
{
  LevelSetApp::registerObjects(factory);
}
