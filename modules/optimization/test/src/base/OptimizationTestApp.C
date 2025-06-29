//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationTestApp.h"
#include "OptimizationApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
OptimizationTestApp::validParams()
{
  InputParameters params = OptimizationApp::validParams();
  return params;
}

registerKnownLabel("OptimizationTestApp");

OptimizationTestApp::OptimizationTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  OptimizationTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

OptimizationTestApp::~OptimizationTestApp() {}

void
OptimizationTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  OptimizationApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"OptimizationTestApp"});
    Registry::registerActionsTo(af, {"OptimizationTestApp"});
  }
}

void
OptimizationTestApp::registerApps()
{
  OptimizationApp::registerApps();
  registerApp(OptimizationTestApp);
}

extern "C" void
OptimizationTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  OptimizationTestApp::registerAll(f, af, s);
}
extern "C" void
OptimizationTestApp__registerApps()
{
  OptimizationTestApp::registerApps();
}
