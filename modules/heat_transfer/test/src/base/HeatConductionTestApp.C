//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionTestApp.h"
#include "HeatConductionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
HeatConductionTestApp::validParams()
{
  InputParameters params = HeatConductionApp::validParams();
  return params;
}

registerKnownLabel("HeatConductionTestApp");

HeatConductionTestApp::HeatConductionTestApp(InputParameters parameters) : MooseApp(parameters)
{
  HeatConductionTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

HeatConductionTestApp::~HeatConductionTestApp() {}

void
HeatConductionTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  HeatConductionApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"HeatConductionTestApp"});
    Registry::registerActionsTo(af, {"HeatConductionTestApp"});
  }
}

void
HeatConductionTestApp::registerApps()
{
  registerApp(HeatConductionApp);
  registerApp(HeatConductionTestApp);
}

void
HeatConductionTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"HeatConductionTestApp"});
}

void
HeatConductionTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"HeatConductionTestApp"});
}

void
HeatConductionTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
HeatConductionTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  HeatConductionTestApp::registerAll(f, af, s);
}
extern "C" void
HeatConductionTestApp__registerApps()
{
  HeatConductionTestApp::registerApps();
}
