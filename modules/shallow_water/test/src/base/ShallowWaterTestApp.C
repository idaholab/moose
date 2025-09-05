//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShallowWaterTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ShallowWaterTestApp::validParams()
{
  InputParameters params = ShallowWaterApp::validParams();
  return params;
}

registerKnownLabel("ShallowWaterTestApp");

ShallowWaterTestApp::ShallowWaterTestApp(const InputParameters & parameters)
  : ShallowWaterApp(parameters)
{
  ShallowWaterTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ShallowWaterTestApp::~ShallowWaterTestApp() {}

void
ShallowWaterTestApp::registerAll(Factory & f,
                                   ActionFactory & af,
                                   Syntax & /*s*/,
                                   bool use_test_objs)
{
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ShallowWaterTestApp"});
    Registry::registerActionsTo(af, {"ShallowWaterTestApp"});
  }
}
void
ShallowWaterTestApp::registerApps()
{
  ShallowWaterApp::registerApps();
  registerApp(ShallowWaterTestApp);
}

extern "C" void
ShallowWaterTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ShallowWaterTestApp::registerAll(f, af, s);
}
extern "C" void
ShallowWaterTestApp__registerApps()
{
  ShallowWaterTestApp::registerApps();
}
