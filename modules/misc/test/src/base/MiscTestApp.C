//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MiscTestApp.h"
#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
MiscTestApp::validParams()
{
  InputParameters params = MiscApp::validParams();
  return params;
}

registerKnownLabel("MiscTestApp");

MiscTestApp::MiscTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  MiscTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

MiscTestApp::~MiscTestApp() {}

void
MiscTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  MiscApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"MiscTestApp"});
    Registry::registerActionsTo(af, {"MiscTestApp"});
  }
}

void
MiscTestApp::registerApps()
{
  MiscApp::registerApps();
  registerApp(MiscTestApp);
}

extern "C" void
MiscTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MiscTestApp::registerAll(f, af, s);
}
extern "C" void
MiscTestApp__registerApps()
{
  MiscTestApp::registerApps();
}
