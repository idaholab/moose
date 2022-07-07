#include "FussionAppApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
FussionAppApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

FussionAppApp::FussionAppApp(InputParameters parameters) : MooseApp(parameters)
{
  FussionAppApp::registerAll(_factory, _action_factory, _syntax);
}

FussionAppApp::~FussionAppApp() {}

void
FussionAppApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"FussionAppApp"});
  Registry::registerActionsTo(af, {"FussionAppApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
FussionAppApp::registerApps()
{
  registerApp(FussionAppApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FussionAppApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FussionAppApp::registerAll(f, af, s);
}
extern "C" void
FussionAppApp__registerApps()
{
  FussionAppApp::registerApps();
}
