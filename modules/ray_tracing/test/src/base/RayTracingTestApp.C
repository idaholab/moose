//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingTestApp.h"
#include "RayTracingApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
RayTracingTestApp::validParams()
{
  InputParameters params = RayTracingApp::validParams();
  return params;
}

registerKnownLabel("RayTracingTestApp");

RayTracingTestApp::RayTracingTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  RayTracingTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

RayTracingTestApp::~RayTracingTestApp() {}

void
RayTracingTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  RayTracingApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"RayTracingTestApp"});
    Registry::registerActionsTo(af, {"RayTracingTestApp"});
  }
}

void
RayTracingTestApp::registerApps()
{
  RayTracingApp::registerApps();
  registerApp(RayTracingTestApp);
}

extern "C" void
RayTracingTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RayTracingTestApp::registerAll(f, af, s);
}
extern "C" void
RayTracingTestApp__registerApps()
{
  RayTracingTestApp::registerApps();
}
