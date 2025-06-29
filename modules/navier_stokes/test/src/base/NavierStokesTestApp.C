//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesTestApp.h"
#include "NavierStokesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
NavierStokesTestApp::validParams()
{
  InputParameters params = NavierStokesApp::validParams();
  return params;
}

registerKnownLabel("NavierStokesTestApp");

NavierStokesTestApp::NavierStokesTestApp(const InputParameters & parameters)
  : NavierStokesApp(parameters)
{
  NavierStokesTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

NavierStokesTestApp::~NavierStokesTestApp() {}

void
NavierStokesTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  NavierStokesApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"NavierStokesTestApp"});
    Registry::registerActionsTo(af, {"NavierStokesTestApp"});
  }
}

void
NavierStokesTestApp::registerApps()
{
  NavierStokesApp::registerApps();
  registerApp(NavierStokesTestApp);
}

extern "C" void
NavierStokesTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  NavierStokesTestApp::registerAll(f, af, s);
}
extern "C" void
NavierStokesTestApp__registerApps()
{
  NavierStokesTestApp::registerApps();
}
