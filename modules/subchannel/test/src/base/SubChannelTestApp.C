#include "SubChannelTestApp.h"
#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
SubChannelTestApp::validParams()
{
  InputParameters params = SubChannelApp::validParams();
  return params;
}

SubChannelTestApp::SubChannelTestApp(InputParameters parameters) : MooseApp(parameters)
{
  SubChannelTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SubChannelTestApp::~SubChannelTestApp() {}

void
SubChannelTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  SubChannelApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SubChannelTestApp"});
    Registry::registerActionsTo(af, {"SubChannelTestApp"});
  }
}

void
SubChannelTestApp::registerApps()
{
  registerApp(SubChannelApp);
  registerApp(SubChannelTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
SubChannelTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelTestApp::registerAll(f, af, s);
}
extern "C" void
SubChannelTestApp__registerApps()
{
  SubChannelTestApp::registerApps();
}
