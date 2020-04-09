//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTestApp.h"
#include "LevelSetApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
LevelSetTestApp::validParams()
{
  InputParameters params = LevelSetApp::validParams();
  return params;
}

registerKnownLabel("LevelSetTestApp");

LevelSetTestApp::LevelSetTestApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());
  LevelSetTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

LevelSetTestApp::~LevelSetTestApp() {}

void
LevelSetTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  LevelSetApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"LevelSetTestApp"});
    Registry::registerActionsTo(af, {"LevelSetTestApp"});
  }
}

void
LevelSetTestApp::registerApps()
{
  registerApp(LevelSetApp);
  registerApp(LevelSetTestApp);
}

void
LevelSetTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"LevelSetTestApp"});
}

void
LevelSetTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"LevelSetTestApp"});
}

void
LevelSetTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
LevelSetTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  LevelSetTestApp::registerAll(f, af, s);
}
extern "C" void
LevelSetTestApp__registerApps()
{
  LevelSetTestApp::registerApps();
}
