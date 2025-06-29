//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RdgTestApp.h"
#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
RdgTestApp::validParams()
{
  InputParameters params = RdgApp::validParams();
  return params;
}

registerKnownLabel("RdgTestApp");

RdgTestApp::RdgTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  RdgTestApp::registerAll(_factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

RdgTestApp::~RdgTestApp() {}

void
RdgTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  RdgApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"RdgTestApp"});
    Registry::registerActionsTo(af, {"RdgTestApp"});
  }
}

void
RdgTestApp::registerApps()
{
  RdgApp::registerApps();
  registerApp(RdgTestApp);
}
extern "C" void
RdgTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RdgTestApp::registerAll(f, af, s);
}
extern "C" void
RdgTestApp__registerApps()
{
  RdgTestApp::registerApps();
}
