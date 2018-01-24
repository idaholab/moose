#include "LevelSetTestApp.h"
#include "LevelSetApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<LevelSetTestApp>()
{
  InputParameters params = validParams<LevelSetApp>();
  return params;
}

LevelSetTestApp::LevelSetTestApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  LevelSetApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LevelSetApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    LevelSetTestApp::registerObjects(_factory);
    LevelSetTestApp::associateSyntax(_syntax, _action_factory);
  }
}

LevelSetTestApp::~LevelSetTestApp() {}

// External entry point for dynamic application loading
extern "C" void
LevelSetTestApp__registerApps()
{
  LevelSetTestApp::registerApps();
}
void
LevelSetTestApp::registerApps()
{
  registerApp(LevelSetApp);
  registerApp(LevelSetTestApp);
}

// External entry point for dynamic object registration
extern "C" void
LevelSetTestApp__registerObjects(Factory & factory)
{
  LevelSetTestApp::registerObjects(factory);
}
void
LevelSetTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
LevelSetTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  LevelSetTestApp::associateSyntax(syntax, action_factory);
}
void
LevelSetTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
