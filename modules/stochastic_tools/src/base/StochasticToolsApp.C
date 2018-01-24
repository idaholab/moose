//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "StateSimTester.h"
#include "StateSimRunner.h"

// Distributions
#include "UniformDistribution.h"
#include "WeibullDistribution.h"
#include "NormalDistribution.h"
#include "LognormalDistribution.h"

// Samplers
#include "MonteCarloSampler.h"
#include "SobolSampler.h"

// VectorPostprocessors
#include "SamplerData.h"
#include "StochasticResults.h"

// MultiApps
#include "SamplerMultiApp.h"

// Transfers
#include "SamplerTransfer.h"
#include "SamplerPostprocessorTransfer.h"

// Controls
#include "SamplerReceiver.h"

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

  Moose::registerExecFlags(_factory);
  StochasticToolsApp::registerExecFlags(_factory);
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
  registerDistribution(WeibullDistribution);
  registerDistribution(NormalDistribution);
  registerDistribution(LognormalDistribution);

  // Samplers
  registerSampler(MonteCarloSampler);
  registerSampler(SobolSampler);

  // VectorPostprocessors
  registerVectorPostprocessor(SamplerData);
  registerVectorPostprocessor(StochasticResults);

  // MultiApps
  registerMultiApp(SamplerMultiApp);

  // Transfers
  registerTransfer(SamplerTransfer);
  registerTransfer(SamplerPostprocessorTransfer);

  // Controls
  registerControl(SamplerReceiver);
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

// External entry point for dynamic execute flag registration
extern "C" void
StochasticToolsApp__registerExecFlags(Factory & factory)
{
  StochasticToolsApp::registerExecFlags(factory);
}
void
StochasticToolsApp::registerExecFlags(Factory & /*factory*/)
{
}
