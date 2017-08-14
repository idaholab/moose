#include "RdgTestApp.h"
#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RdgTestApp>()
{
  InputParameters params = validParams<RdgApp>();
  return params;
}

RdgTestApp::RdgTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RdgApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RdgApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    RdgTestApp::registerObjects(_factory);
    RdgTestApp::associateSyntax(_syntax, _action_factory);
  }
}

RdgTestApp::~RdgTestApp() {}

// External entry point for dynamic application loading
extern "C" void
RdgTestApp__registerApps()
{
  RdgTestApp::registerApps();
}
void
RdgTestApp::registerApps()
{
  registerApp(RdgApp);
  registerApp(RdgTestApp);
}

// External entry point for dynamic object registration
extern "C" void
RdgTestApp__registerObjects(Factory & factory)
{
  RdgTestApp::registerObjects(factory);
}
void
RdgTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
RdgTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RdgTestApp::associateSyntax(syntax, action_factory);
}
void
RdgTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
