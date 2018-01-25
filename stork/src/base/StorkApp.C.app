#include "StorkApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<StorkApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

StorkApp::StorkApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  StorkApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  StorkApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ModulesApp::registerExecFlags(_factory);
  StorkApp::registerExecFlags(_factory);
}

StorkApp::~StorkApp() {}

void
StorkApp::registerApps()
{
  registerApp(StorkApp);
}

void
StorkApp::registerObjects(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new production objects here! */
}

void
StorkApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  /* Uncomment Syntax and ActionFactory parameters and register your new production objects here! */
}

void
StorkApp::registerObjectDepends(Factory & /*factory*/)
{
}

void
StorkApp::associateSyntaxDepends(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

void
StorkApp::registerExecFlags(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new execution flags here! */
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
StorkApp__registerApps()
{
  StorkApp::registerApps();
}

extern "C" void
StorkApp__registerObjects(Factory & factory)
{
  StorkApp::registerObjects(factory);
}

extern "C" void
StorkApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  StorkApp::associateSyntax(syntax, action_factory);
}

extern "C" void
StorkApp__registerExecFlags(Factory & factory)
{
  StorkApp::registerExecFlags(factory);
}
