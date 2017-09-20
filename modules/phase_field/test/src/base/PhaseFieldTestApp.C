#include "PhaseFieldTestApp.h"
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "GaussContForcing.h"

template <>
InputParameters
validParams<PhaseFieldTestApp>()
{
  InputParameters params = validParams<PhaseFieldApp>();
  return params;
}

PhaseFieldTestApp::PhaseFieldTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    PhaseFieldTestApp::registerObjects(_factory);
    PhaseFieldTestApp::associateSyntax(_syntax, _action_factory);
  }
}

PhaseFieldTestApp::~PhaseFieldTestApp() {}

// External entry point for dynamic application loading
extern "C" void
PhaseFieldTestApp__registerApps()
{
  PhaseFieldTestApp::registerApps();
}
void
PhaseFieldTestApp::registerApps()
{
  registerApp(PhaseFieldApp);
  registerApp(PhaseFieldTestApp);
}

// External entry point for dynamic object registration
extern "C" void
PhaseFieldTestApp__registerObjects(Factory & factory)
{
  PhaseFieldTestApp::registerObjects(factory);
}
void
PhaseFieldTestApp::registerObjects(Factory & factory)
{
  registerKernel(GaussContForcing);
}

// External entry point for dynamic syntax association
extern "C" void
PhaseFieldTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PhaseFieldTestApp::associateSyntax(syntax, action_factory);
}
void
PhaseFieldTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
