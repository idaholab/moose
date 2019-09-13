#include "sub_channelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<sub_channelApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

sub_channelApp::sub_channelApp(InputParameters parameters) : MooseApp(parameters)
{
  sub_channelApp::registerAll(_factory, _action_factory, _syntax);
}

sub_channelApp::~sub_channelApp() {}

void
sub_channelApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"sub_channelApp"});
  Registry::registerActionsTo(af, {"sub_channelApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
sub_channelApp::registerApps()
{
  registerApp(sub_channelApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
sub_channelApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  sub_channelApp::registerAll(f, af, s);
}
extern "C" void
sub_channelApp__registerApps()
{
  sub_channelApp::registerApps();
}
