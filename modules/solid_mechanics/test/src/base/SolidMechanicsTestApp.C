#include "SolidMechanicsTestApp.h"
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidMechanicsTestApp>()
{
  InputParameters params = validParams<SolidMechanicsApp>();
  return params;
}

SolidMechanicsTestApp::SolidMechanicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  SolidMechanicsApp::registerObjectDepends(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntaxDepends(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    SolidMechanicsTestApp::registerObjects(_factory);
    SolidMechanicsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

SolidMechanicsTestApp::~SolidMechanicsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidMechanicsTestApp__registerApps()
{
  SolidMechanicsTestApp::registerApps();
}
void
SolidMechanicsTestApp::registerApps()
{
  registerApp(SolidMechanicsApp);
  registerApp(SolidMechanicsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
SolidMechanicsTestApp__registerObjects(Factory & factory)
{
  SolidMechanicsTestApp::registerObjects(factory);
}
void
SolidMechanicsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
SolidMechanicsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsTestApp::associateSyntax(syntax, action_factory);
}
void
SolidMechanicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
