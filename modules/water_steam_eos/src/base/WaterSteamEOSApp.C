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

// Initialize static member variables
bool WaterSteamEOSApp::_registered_objects = false;
bool WaterSteamEOSApp::_associated_syntax = false;

template<>
InputParameters validParams<WaterSteamEOSApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

WaterSteamEOSApp::WaterSteamEOSApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  WaterSteamEOSApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  WaterSteamEOSApp::associateSyntax(_syntax, _action_factory);
}

WaterSteamEOSApp::~WaterSteamEOSApp()
{
}

// External entry point for dynamic application loading
extern "C" void WaterSteamEOSApp__registerApps() { WaterSteamEOSApp::registerApps(); }
void
WaterSteamEOSApp::registerApps()
{
  registerApp(WaterSteamEOSApp);
}

// External entry point for dynamic object registration
extern "C" void WaterSteamEOSApp__registerObjects(Factory & factory) { WaterSteamEOSApp::registerObjects(factory); }
void
WaterSteamEOSApp::registerObjects(Factory & /*factory*/)
{
  if (_registered_objects)
    return;
  _registered_objects = true;

}

// External entry point for dynamic syntax association
extern "C" void WaterSteamEOSApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { WaterSteamEOSApp::associateSyntax(syntax, action_factory); }
void
WaterSteamEOSApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  if (_associated_syntax)
    return;
  _associated_syntax = true;

}
