//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticToolsTestApp.h"
#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
StochasticToolsTestApp::validParams()
{
  InputParameters params = StochasticToolsApp::validParams();
  return params;
}

registerKnownLabel("StochasticToolsTestApp");

StochasticToolsTestApp::StochasticToolsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  StochasticToolsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

StochasticToolsTestApp::~StochasticToolsTestApp() {}

void
StochasticToolsTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  StochasticToolsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"StochasticToolsTestApp"});
    Registry::registerActionsTo(af, {"StochasticToolsTestApp"});
  }
}

void
StochasticToolsTestApp::registerApps()
{
  registerApp(StochasticToolsApp);
  registerApp(StochasticToolsTestApp);
}

void
StochasticToolsTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"StochasticToolsTestApp"});
}

void
StochasticToolsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"StochasticToolsTestApp"});
}

void
StochasticToolsTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
StochasticToolsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  StochasticToolsTestApp::registerAll(f, af, s);
}
extern "C" void
StochasticToolsTestApp__registerApps()
{
  StochasticToolsTestApp::registerApps();
}
