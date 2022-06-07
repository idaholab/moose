#include "SolidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidPropertiesApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  return params;
}

SolidPropertiesApp::SolidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  SolidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

SolidPropertiesApp::~SolidPropertiesApp() {}

void
SolidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"SolidPropertiesApp"});
  Registry::registerActionsTo(af, {"SolidPropertiesApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
SolidPropertiesApp::registerApps()
{
  registerApp(SolidPropertiesApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
SolidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
SolidPropertiesApp__registerApps()
{
  SolidPropertiesApp::registerApps();
}
