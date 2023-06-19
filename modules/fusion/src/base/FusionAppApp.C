#include "FusionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"
#include "ThermalHydraulicsApp.h"

InputParameters
FusionApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

FusionApp::FusionApp(InputParameters parameters) : MooseApp(parameters)
{
  FusionApp::registerAll(_factory, _action_factory, _syntax);
}

FusionApp::~FusionApp() {}

void
FusionApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"FusionApp"});
  Registry::registerActionsTo(af, {"FusionApp"});

  /* register custom execute flags, action syntax, etc. here */
  ThermalHydraulicsApp::registerAll(f, af, syntax);
}

void
FusionApp::registerApps()
{
  registerApp(FusionApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FusionApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FusionApp::registerAll(f, af, s);
}
extern "C" void
FusionApp__registerApps()
{
  FusionApp::registerApps();
}
