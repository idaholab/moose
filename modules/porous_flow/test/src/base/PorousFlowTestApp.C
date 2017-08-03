#include "PorousFlowTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<PorousFlowTestApp>()
{
  InputParameters params = validParams<PorousFlowApp>();
  return params;
}

PorousFlowTestApp::PorousFlowTestApp(InputParameters parameters) : PorousFlowApp(parameters)
{
  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    PorousFlowTestApp::registerObjects(_factory);
    PorousFlowTestApp::associateSyntax(_syntax, _action_factory);
  }
}

PorousFlowTestApp::~PorousFlowTestApp() {}

// External entry point for dynamic application loading
extern "C" void
PorousFlowTestApp__registerApps()
{
  PorousFlowTestApp::registerApps();
}
void
PorousFlowTestApp::registerApps()
{
  registerApp(PorousFlowApp);
  registerApp(PorousFlowTestApp);
}

// External entry point for dynamic object registration
extern "C" void
PorousFlowTestApp__registerObjects(Factory & factory)
{
  PorousFlowTestApp::registerObjects(factory);
}
void
PorousFlowTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
PorousFlowTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PorousFlowTestApp::associateSyntax(syntax, action_factory);
}
void
PorousFlowTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
