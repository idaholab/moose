#include "FluidStructureInteractionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FluidStructureInteractionApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  return params;
}

FluidStructureInteractionApp::FluidStructureInteractionApp(InputParameters parameters)
  : MooseApp(parameters)
{
  FluidStructureInteractionApp::registerAll(_factory, _action_factory, _syntax);
}

FluidStructureInteractionApp::~FluidStructureInteractionApp() {}

void
FluidStructureInteractionApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"FluidStructureInteractionApp"});
  Registry::registerActionsTo(af, {"FluidStructureInteractionApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
FluidStructureInteractionApp::registerApps()
{
  registerApp(FluidStructureInteractionApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FluidStructureInteractionApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidStructureInteractionApp::registerAll(f, af, s);
}
extern "C" void
FluidStructureInteractionApp__registerApps()
{
  FluidStructureInteractionApp::registerApps();
}
