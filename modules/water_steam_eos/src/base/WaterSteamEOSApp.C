/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "WaterSteamEOSApp.h"
#include "Moose.h"
#include "AppFactory.h"

template<>
InputParameters validParams<WaterSteamEOSApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = true;
  params.set<bool>("use_legacy_uo_aux_computation") = false;

  return params;
}

WaterSteamEOSApp::WaterSteamEOSApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  WaterSteamEOSApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  WaterSteamEOSApp::associateSyntax(_syntax, _action_factory);
}

WaterSteamEOSApp::~WaterSteamEOSApp()
{
}

void
WaterSteamEOSApp::registerApps()
{
  registerApp(WaterSteamEOSApp);
}

void
WaterSteamEOSApp::registerObjects(Factory & /*factory*/)
{
}

void
WaterSteamEOSApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
