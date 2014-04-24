#include "WaterSteamEOSApp.h"
#include "Moose.h"
#include "AppFactory.h"

template<>
InputParameters validParams<WaterSteamEOSApp>()
{
  InputParameters params = validParams<MooseApp>();
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
