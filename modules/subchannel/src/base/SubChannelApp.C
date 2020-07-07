#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
SubChannelApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

SubChannelApp::SubChannelApp(InputParameters parameters) : MooseApp(parameters)
{
  SubChannelApp::registerAll(_factory, _action_factory, _syntax);
}

SubChannelApp::~SubChannelApp() {}

void
SubChannelApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"SubChannelApp"});
  Registry::registerActionsTo(af, {"SubChannelApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
SubChannelApp::registerApps()
{
  registerApp(SubChannelApp);
}
/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
SubChannelApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelApp::registerAll(f, af, s);
}
extern "C" void
SubChannelApp__registerApps()
{
  SubChannelApp::registerApps();
}
