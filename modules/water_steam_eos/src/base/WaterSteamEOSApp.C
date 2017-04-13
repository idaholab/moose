/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

  Moose::registerExecFlags();
  WaterSteamEOSApp::registerExecFlags();
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

void
WaterSteamEOSApp::registerExecFlags()
{
}
