//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesTestApp.h"
#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FluidPropertiesTestApp::validParams()
{
  InputParameters params = FluidPropertiesApp::validParams();
  return params;
}

registerKnownLabel("FluidPropertiesTestApp");

FluidPropertiesTestApp::FluidPropertiesTestApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  FluidPropertiesTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

FluidPropertiesTestApp::~FluidPropertiesTestApp() {}

void
FluidPropertiesTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  FluidPropertiesApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FluidPropertiesTestApp"});
    Registry::registerActionsTo(af, {"FluidPropertiesTestApp"});
  }
}

void
FluidPropertiesTestApp::registerApps()
{
  FluidPropertiesApp::registerApps();
  registerApp(FluidPropertiesTestApp);
}

extern "C" void
FluidPropertiesTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidPropertiesTestApp::registerAll(f, af, s);
}
extern "C" void
FluidPropertiesTestApp__registerApps()
{
  FluidPropertiesTestApp::registerApps();
}
