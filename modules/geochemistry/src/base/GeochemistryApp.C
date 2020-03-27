#include "GeochemistryApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
GeochemistryApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  return params;
}

GeochemistryApp::GeochemistryApp(InputParameters parameters) : MooseApp(parameters)
{
  GeochemistryApp::registerAll(_factory, _action_factory, _syntax);
}

GeochemistryApp::~GeochemistryApp() {}

void
GeochemistryApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"GeochemistryApp"});
  Registry::registerActionsTo(af, {"GeochemistryApp"});

  /* register custom execute flags, action syntax, etc. here */
  s.registerActionSyntax("AddGeochemicalModelInterrogatorAction", "GeochemicalModelInterrogator");
}

void
GeochemistryApp::registerApps()
{
  registerApp(GeochemistryApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
GeochemistryApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  GeochemistryApp::registerAll(f, af, s);
}
extern "C" void
GeochemistryApp__registerApps()
{
  GeochemistryApp::registerApps();
}
