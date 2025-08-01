//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsTestApp.h"
#include "PeridynamicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
PeridynamicsTestApp::validParams()
{
  InputParameters params = PeridynamicsApp::validParams();

  return params;
}

registerKnownLabel("PeridynamicsTestApp");

PeridynamicsTestApp::PeridynamicsTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  PeridynamicsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

PeridynamicsTestApp::~PeridynamicsTestApp() {}

void
PeridynamicsTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  PeridynamicsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"PeridynamicsTestApp"});
    Registry::registerActionsTo(af, {"PeridynamicsTestApp"});
  }
}

void
PeridynamicsTestApp::registerApps()
{
  PeridynamicsApp::registerApps();
  registerApp(PeridynamicsTestApp);
}

extern "C" void
PeridynamicsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PeridynamicsTestApp::registerAll(f, af, s);
}

extern "C" void
PeridynamicsTestApp__registerApps()
{
  PeridynamicsTestApp::registerApps();
}
