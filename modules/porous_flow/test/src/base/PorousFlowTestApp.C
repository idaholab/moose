//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowTestApp.h"
#include "PorousFlowApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
PorousFlowTestApp::validParams()
{
  InputParameters params = PorousFlowApp::validParams();
  return params;
}

registerKnownLabel("PorousFlowTestApp");

PorousFlowTestApp::PorousFlowTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  PorousFlowTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

PorousFlowTestApp::~PorousFlowTestApp() {}

void
PorousFlowTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  PorousFlowApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"PorousFlowTestApp"});
    Registry::registerActionsTo(af, {"PorousFlowTestApp"});
  }
}

void
PorousFlowTestApp::registerApps()
{
  PorousFlowApp::registerApps();
  registerApp(PorousFlowTestApp);
}

extern "C" void
PorousFlowTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PorousFlowTestApp::registerAll(f, af, s);
}
extern "C" void
PorousFlowTestApp__registerApps()
{
  PorousFlowTestApp::registerApps();
}
