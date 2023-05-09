#include "MeshingTutorialApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
MeshingTutorialApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

MeshingTutorialApp::MeshingTutorialApp(InputParameters parameters) : MooseApp(parameters)
{
  MeshingTutorialApp::registerAll(_factory, _action_factory, _syntax);
}

MeshingTutorialApp::~MeshingTutorialApp() {}

void
MeshingTutorialApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"MeshingTutorialApp"});
  Registry::registerActionsTo(af, {"MeshingTutorialApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
MeshingTutorialApp::registerApps()
{
  registerApp(MeshingTutorialApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
MeshingTutorialApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MeshingTutorialApp::registerAll(f, af, s);
}
extern "C" void
MeshingTutorialApp__registerApps()
{
  MeshingTutorialApp::registerApps();
}
