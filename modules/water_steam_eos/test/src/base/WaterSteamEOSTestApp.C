//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "WaterSteamEOSTestApp.h"
#include "WaterSteamEOSApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<WaterSteamEOSTestApp>()
{
  InputParameters params = validParams<WaterSteamEOSApp>();
  return params;
}

WaterSteamEOSTestApp::WaterSteamEOSTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  WaterSteamEOSApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  WaterSteamEOSApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  WaterSteamEOSApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    WaterSteamEOSTestApp::registerObjects(_factory);
    WaterSteamEOSTestApp::associateSyntax(_syntax, _action_factory);
  }
}

WaterSteamEOSTestApp::~WaterSteamEOSTestApp() {}

// External entry point for dynamic application loading
extern "C" void
WaterSteamEOSTestApp__registerApps()
{
  WaterSteamEOSTestApp::registerApps();
}
void
WaterSteamEOSTestApp::registerApps()
{
  registerApp(WaterSteamEOSApp);
  registerApp(WaterSteamEOSTestApp);
}

// External entry point for dynamic object registration
extern "C" void
WaterSteamEOSTestApp__registerObjects(Factory & factory)
{
  WaterSteamEOSTestApp::registerObjects(factory);
}
void
WaterSteamEOSTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
WaterSteamEOSTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  WaterSteamEOSTestApp::associateSyntax(syntax, action_factory);
}
void
WaterSteamEOSTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
WaterSteamEOSTestApp__registerExecFlags(Factory & factory)
{
  WaterSteamEOSTestApp::registerExecFlags(factory);
}
void
WaterSteamEOSTestApp::registerExecFlags(Factory & /*factory*/)
{
}
