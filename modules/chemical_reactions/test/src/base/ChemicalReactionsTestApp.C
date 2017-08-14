#include "ChemicalReactionsTestApp.h"
#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<ChemicalReactionsTestApp>()
{
  InputParameters params = validParams<ChemicalReactionsApp>();
  return params;
}

ChemicalReactionsTestApp::ChemicalReactionsTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    ChemicalReactionsTestApp::registerObjects(_factory);
    ChemicalReactionsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

ChemicalReactionsTestApp::~ChemicalReactionsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsTestApp__registerApps()
{
  ChemicalReactionsTestApp::registerApps();
}
void
ChemicalReactionsTestApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
  registerApp(ChemicalReactionsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
ChemicalReactionsTestApp__registerObjects(Factory & factory)
{
  ChemicalReactionsTestApp::registerObjects(factory);
}
void
ChemicalReactionsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
ChemicalReactionsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsTestApp::associateSyntax(syntax, action_factory);
}
void
ChemicalReactionsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
