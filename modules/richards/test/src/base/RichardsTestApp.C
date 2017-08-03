#include "RichardsTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RichardsTestApp>()
{
  InputParameters params = validParams<RichardsApp>();
  return params;
}

RichardsTestApp::RichardsTestApp(InputParameters parameters) : RichardsApp(parameters)
{
  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    RichardsTestApp::registerObjects(_factory);
    RichardsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

RichardsTestApp::~RichardsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
RichardsTestApp__registerApps()
{
  RichardsTestApp::registerApps();
}
void
RichardsTestApp::registerApps()
{
  registerApp(RichardsApp);
  registerApp(RichardsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
RichardsTestApp__registerObjects(Factory & factory)
{
  RichardsTestApp::registerObjects(factory);
}
void
RichardsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
RichardsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RichardsTestApp::associateSyntax(syntax, action_factory);
}
void
RichardsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
