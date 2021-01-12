#include "MultiAppTutApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
MultiAppTutApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  return params;
}

MultiAppTutApp::MultiAppTutApp(InputParameters parameters) : MooseApp(parameters)
{
  MultiAppTutApp::registerAll(_factory, _action_factory, _syntax);
}

MultiAppTutApp::~MultiAppTutApp() {}

void
MultiAppTutApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"MultiAppTutApp"});
  Registry::registerActionsTo(af, {"MultiAppTutApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
MultiAppTutApp::registerApps()
{
  registerApp(MultiAppTutApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
MultiAppTutApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MultiAppTutApp::registerAll(f, af, s);
}
extern "C" void
MultiAppTutApp__registerApps()
{
  MultiAppTutApp::registerApps();
}
