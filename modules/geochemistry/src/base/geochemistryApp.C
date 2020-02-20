#include "geochemistryApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
geochemistryApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  return params;
}

geochemistryApp::geochemistryApp(InputParameters parameters) : MooseApp(parameters)
{
  geochemistryApp::registerAll(_factory, _action_factory, _syntax);
}

geochemistryApp::~geochemistryApp() {}

void
geochemistryApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"geochemistryApp"});
  Registry::registerActionsTo(af, {"geochemistryApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
geochemistryApp::registerApps()
{
  registerApp(geochemistryApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
geochemistryApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  geochemistryApp::registerAll(f, af, s);
}
extern "C" void
geochemistryApp__registerApps()
{
  geochemistryApp::registerApps();
}
