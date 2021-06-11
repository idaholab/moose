#include "BabblerApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
BabblerApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

BabblerApp::BabblerApp(InputParameters parameters) : MooseApp(parameters)
{
  BabblerApp::registerAll(_factory, _action_factory, _syntax);
}

BabblerApp::~BabblerApp() {}

void
BabblerApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"BabblerApp"});
  Registry::registerActionsTo(af, {"BabblerApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
BabblerApp::registerApps()
{
  registerApp(BabblerApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
BabblerApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  BabblerApp::registerAll(f, af, s);
}
extern "C" void
BabblerApp__registerApps()
{
  BabblerApp::registerApps();
}
