#include "StorkApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

template<>
InputParameters validParams<StorkApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

StorkApp::StorkApp(InputParameters parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  StorkApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  StorkApp::associateSyntax(_syntax, _action_factory);
}

StorkApp::~StorkApp()
{
}

// External entry point for dynamic application loading
extern "C" void StorkApp__registerApps() { StorkApp::registerApps(); }
void
StorkApp::registerApps()
{
  registerApp(StorkApp);
}

// External entry point for dynamic object registration
extern "C" void StorkApp__registerObjects(Factory & factory) { StorkApp::registerObjects(factory); }
void
StorkApp::registerObjects(Factory & factory)
{
}

// External entry point for dynamic syntax association
extern "C" void StorkApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { StorkApp::associateSyntax(syntax, action_factory); }
void
StorkApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
