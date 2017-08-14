#include "FluidPropertiesTestApp.h"
#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<FluidPropertiesTestApp>()
{
  InputParameters params = validParams<FluidPropertiesApp>();
  return params;
}

FluidPropertiesTestApp::FluidPropertiesTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FluidPropertiesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FluidPropertiesApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    FluidPropertiesTestApp::registerObjects(_factory);
    FluidPropertiesTestApp::associateSyntax(_syntax, _action_factory);
  }
}

FluidPropertiesTestApp::~FluidPropertiesTestApp() {}

// External entry point for dynamic application loading
extern "C" void
FluidPropertiesTestApp__registerApps()
{
  FluidPropertiesTestApp::registerApps();
}
void
FluidPropertiesTestApp::registerApps()
{
  registerApp(FluidPropertiesApp);
  registerApp(FluidPropertiesTestApp);
}

// External entry point for dynamic object registration
extern "C" void
FluidPropertiesTestApp__registerObjects(Factory & factory)
{
  FluidPropertiesTestApp::registerObjects(factory);
}
void
FluidPropertiesTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
FluidPropertiesTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesTestApp::associateSyntax(syntax, action_factory);
}
void
FluidPropertiesTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
