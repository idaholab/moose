//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaterSteamEOSApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<WaterSteamEOSApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

WaterSteamEOSApp::WaterSteamEOSApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  WaterSteamEOSApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  WaterSteamEOSApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  WaterSteamEOSApp::registerExecFlags(_factory);
}

WaterSteamEOSApp::~WaterSteamEOSApp() {}

// External entry point for dynamic application loading
extern "C" void
WaterSteamEOSApp__registerApps()
{
  WaterSteamEOSApp::registerApps();
}
void
WaterSteamEOSApp::registerApps()
{
  registerApp(WaterSteamEOSApp);
}

// External entry point for dynamic object registration
extern "C" void
WaterSteamEOSApp__registerObjects(Factory & factory)
{
  WaterSteamEOSApp::registerObjects(factory);
}
void
WaterSteamEOSApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
WaterSteamEOSApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  WaterSteamEOSApp::associateSyntax(syntax, action_factory);
}
void
WaterSteamEOSApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
WaterSteamEOSApp__registerExecFlags(Factory & factory)
{
  WaterSteamEOSApp::registerExecFlags(factory);
}
void
WaterSteamEOSApp::registerExecFlags(Factory & /*factory*/)
{
}
