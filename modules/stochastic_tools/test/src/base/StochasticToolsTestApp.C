#include "StochasticToolsTestApp.h"
#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "TestDistributionPostprocessor.h"
#include "TestSampler.h"

template <>
InputParameters
validParams<StochasticToolsTestApp>()
{
  InputParameters params = validParams<StochasticToolsApp>();
  return params;
}

StochasticToolsTestApp::StochasticToolsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  StochasticToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  StochasticToolsApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    StochasticToolsTestApp::registerObjects(_factory);
    StochasticToolsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

StochasticToolsTestApp::~StochasticToolsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
StochasticToolsTestApp__registerApps()
{
  StochasticToolsTestApp::registerApps();
}
void
StochasticToolsTestApp::registerApps()
{
  registerApp(StochasticToolsApp);
  registerApp(StochasticToolsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
StochasticToolsTestApp__registerObjects(Factory & factory)
{
  StochasticToolsTestApp::registerObjects(factory);
}
void
StochasticToolsTestApp::registerObjects(Factory & factory)
{
  registerPostprocessor(TestDistributionPostprocessor);
  registerUserObject(TestSampler);
}

// External entry point for dynamic syntax association
extern "C" void
StochasticToolsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  StochasticToolsTestApp::associateSyntax(syntax, action_factory);
}
void
StochasticToolsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
