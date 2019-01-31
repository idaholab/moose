#include "THMApp.h"
#include "THMSyntax.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<THMApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

THMApp::THMApp(InputParameters parameters) : MooseApp(parameters)
{
  THMApp::registerAll(_factory, _action_factory, _syntax);
}

void
THMApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"THMApp"});
  Registry::registerActionsTo(af, {"THMApp"});

  /* register custom execute flags, action syntax, etc. here */
  THM::associateSyntax(s);
  THM::registerActions(s);
}

void
THMApp::registerApps()
{
  registerApp(THMApp);
}

//
// Dynamic Library Entry Points - DO NOT MODIFY
//
extern "C" void
THMApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  THMApp::registerAll(f, af, s);
}

extern "C" void
THMApp__registerApps()
{
  THMApp::registerApps();
}
