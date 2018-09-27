//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SolidMechanicsTestApp.h"
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidMechanicsTestApp>()
{
  InputParameters params = validParams<SolidMechanicsApp>();
  return params;
}

registerKnownLabel("SolidMechanicsTestApp");

SolidMechanicsTestApp::SolidMechanicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  SolidMechanicsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SolidMechanicsTestApp::~SolidMechanicsTestApp() {}

void
SolidMechanicsTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  SolidMechanicsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SolidMechanicsTestApp"});
    Registry::registerActionsTo(af, {"SolidMechanicsTestApp"});
  }
}

void
SolidMechanicsTestApp::registerApps()
{
  registerApp(SolidMechanicsApp);
  registerApp(SolidMechanicsTestApp);
}

void
SolidMechanicsTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"SolidMechanicsTestApp"});
}

void
SolidMechanicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"SolidMechanicsTestApp"});
}

void
SolidMechanicsTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
SolidMechanicsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidMechanicsTestApp::registerAll(f, af, s);
}
extern "C" void
SolidMechanicsTestApp__registerApps()
{
  SolidMechanicsTestApp::registerApps();
}
