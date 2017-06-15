#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "StateSimTester.h"
#include "StateSimRunner.h"

// Distributions
#include "UniformDistribution.h"
#ifdef LIBMESH_HAVE_BOOST
#include "WeibullDistribution.h"
#endif

// Samplers
#include "MonteCarloSampler.h"

// VectorPostprocessors
#include "SamplerData.h"

// for test purpose only
#include "TestDistributionPostprocessor.h"
#include "TestSampler.h"

template <>
InputParameters
validParams<StochasticToolsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

StochasticToolsApp::StochasticToolsApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  StochasticToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  StochasticToolsApp::associateSyntax(_syntax, _action_factory);
}

StochasticToolsApp::~StochasticToolsApp() {}

// External entry point for dynamic application loading
extern "C" void
StochasticToolsApp__registerApps()
{
  StochasticToolsApp::registerApps();
}
void
StochasticToolsApp::registerApps()
{
  registerApp(StochasticToolsApp);
}

// External entry point for dynamic object registration
extern "C" void
StochasticToolsApp__registerObjects(Factory & factory)
{
  StochasticToolsApp::registerObjects(factory);
}
void
StochasticToolsApp::registerObjects(Factory & factory)
{
  registerUserObject(StateSimRunner);
  registerPostprocessor(StateSimTester);

  // Distributions
  registerDistribution(UniformDistribution);
#ifdef LIBMESH_HAVE_BOOST
  registerDistribution(WeibullDistribution);
#endif

  // Samplers
  registerSampler(MonteCarloSampler);

  // VectorPostprocessors
  registerVectorPostprocessor(SamplerData);

  // for test purpose only
  registerPostprocessor(TestDistributionPostprocessor);
  registerUserObject(TestSampler);
}

// External entry point for dynamic syntax association
extern "C" void
StochasticToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  StochasticToolsApp::associateSyntax(syntax, action_factory);
}
void
StochasticToolsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
