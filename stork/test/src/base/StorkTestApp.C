#include "StorkTestApp.h"
#include "StorkApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<StorkTestApp>()
{
  InputParameters params = validParams<StorkApp>();
  return params;
}

StorkTestApp::StorkTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  StorkApp::registerObjectDepends(_factory);
  StorkApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  StorkApp::associateSyntaxDepends(_syntax, _action_factory);
  StorkApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    StorkTestApp::registerObjects(_factory);
    StorkTestApp::associateSyntax(_syntax, _action_factory);
  }
}

StorkTestApp::~StorkTestApp() {}

// External entry point for dynamic application loading
extern "C" void
StorkTestApp__registerApps()
{
  StorkTestApp::registerApps();
}
void
StorkTestApp::registerApps()
{
  registerApp(StorkApp);
  registerApp(StorkTestApp);
}

// External entry point for dynamic object registration
extern "C" void
StorkTestApp__registerObjects(Factory & factory)
{
  StorkTestApp::registerObjects(factory);
}
void
StorkTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
StorkTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  StorkTestApp::associateSyntax(syntax, action_factory);
}
void
StorkTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
