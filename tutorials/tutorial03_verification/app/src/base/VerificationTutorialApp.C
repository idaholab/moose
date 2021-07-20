#include "VerificationTutorialApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
VerificationTutorialApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

VerificationTutorialApp::VerificationTutorialApp(InputParameters parameters) : MooseApp(parameters)
{
  VerificationTutorialApp::registerAll(_factory, _action_factory, _syntax);
}

VerificationTutorialApp::~VerificationTutorialApp() {}

void
VerificationTutorialApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"VerificationTutorialApp"});
  Registry::registerActionsTo(af, {"VerificationTutorialApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
VerificationTutorialApp::registerApps()
{
  registerApp(VerificationTutorialApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
VerificationTutorialApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  VerificationTutorialApp::registerAll(f, af, s);
}
extern "C" void
VerificationTutorialApp__registerApps()
{
  VerificationTutorialApp::registerApps();
}
