#include "FunctionalExpansionToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<FunctionalExpansionToolsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

FunctionalExpansionToolsApp::FunctionalExpansionToolsApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FunctionalExpansionToolsApp::registerObjectDepends(_factory);
  FunctionalExpansionToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntaxDepends(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntax(_syntax, _action_factory);
}

FunctionalExpansionToolsApp::~FunctionalExpansionToolsApp() {}


void
FunctionalExpansionToolsApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
}

void
FunctionalExpansionToolsApp::registerObjects(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new objects here! */
}

void
FunctionalExpansionToolsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  /* Uncomment Syntax and ActionFactory parameters and register your new objects here! */
}

void
FunctionalExpansionToolsApp::registerObjectDepends(Factory & /*factory*/)
{
}

void
FunctionalExpansionToolsApp::associateSyntaxDepends(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}


/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FunctionalExpansionToolsApp__registerApps()
{
  FunctionalExpansionToolsApp::registerApps();
}

extern "C" void
FunctionalExpansionToolsApp__registerObjects(Factory & factory)
{
  FunctionalExpansionToolsApp::registerObjects(factory);
}

extern "C" void
FunctionalExpansionToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
}
