#include "TensorMechanicsTestApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<TensorMechanicsTestApp>()
{
  InputParameters params = validParams<TensorMechanicsApp>();
  return params;
}

TensorMechanicsTestApp::TensorMechanicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    TensorMechanicsTestApp::registerObjects(_factory);
    TensorMechanicsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

TensorMechanicsTestApp::~TensorMechanicsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
TensorMechanicsTestApp__registerApps()
{
  TensorMechanicsTestApp::registerApps();
}
void
TensorMechanicsTestApp::registerApps()
{
  registerApp(TensorMechanicsApp);
  registerApp(TensorMechanicsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
TensorMechanicsTestApp__registerObjects(Factory & factory)
{
  TensorMechanicsTestApp::registerObjects(factory);
}
void
TensorMechanicsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
TensorMechanicsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsTestApp::associateSyntax(syntax, action_factory);
}
void
TensorMechanicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
