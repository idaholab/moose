#include "StorkApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
StorkApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

StorkApp::StorkApp(InputParameters parameters) : MooseApp(parameters)
{
  StorkApp::registerAll(_factory, _action_factory, _syntax);
}

StorkApp::~StorkApp() {}

void
StorkApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAllObjects<StorkApp>(f, af, syntax);
  Registry::registerObjectsTo(f, {"StorkApp"});
  Registry::registerActionsTo(af, {"StorkApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
StorkApp::registerApps()
{
  registerApp(StorkApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
StorkApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  StorkApp::registerAll(f, af, s);
}
extern "C" void
StorkApp__registerApps()
{
  StorkApp::registerApps();
}
