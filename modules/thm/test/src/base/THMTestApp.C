#include "THMTestApp.h"
#include "THMApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

template <>
InputParameters
validParams<THMTestApp>()
{
  InputParameters params = validParams<THMApp>();
  return params;
}

THMTestApp::THMTestApp(InputParameters parameters) : MooseApp(parameters)
{
  THMTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

THMTestApp::~THMTestApp() {}

void
THMTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  THMApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"THMTestApp"});
    Registry::registerActionsTo(af, {"THMTestApp"});
  }
}

void
THMTestApp::registerApps()
{
  registerApp(THMApp);
  registerApp(THMTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
THMTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  THMTestApp::registerAll(f, af, s);
}

extern "C" void
THMTestApp__registerApps()
{
  THMTestApp::registerApps();
}
