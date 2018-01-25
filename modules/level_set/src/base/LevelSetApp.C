//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "LevelSetApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "LevelSetTypes.h"

// Kernels
#include "LevelSetAdvection.h"
#include "LevelSetAdvectionSUPG.h"
#include "LevelSetTimeDerivativeSUPG.h"
#include "LevelSetForcingFunctionSUPG.h"
#include "LevelSetOlssonReinitialization.h"

// Functions
#include "LevelSetOlssonBubble.h"
#include "LevelSetOlssonVortex.h"

// Postprocessors
#include "LevelSetCFLCondition.h"
#include "LevelSetVolume.h"
#include "LevelSetOlssonTerminator.h"

// Problems
#include "LevelSetProblem.h"
#include "LevelSetReinitializationProblem.h"

// MultiApps
#include "LevelSetReinitializationMultiApp.h"

// Transfers
#include "LevelSetMeshRefinementTransfer.h"

template <>
InputParameters
validParams<LevelSetApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.addClassDescription(
      "Application containing object necessary to solve the level set equation.");
  return params;
}

LevelSetApp::LevelSetApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  LevelSetApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LevelSetApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  LevelSetApp::registerExecFlags(_factory);
}

void
LevelSetApp::registerApps()
{
  registerApp(LevelSetApp);
}

void
LevelSetApp::registerObjects(Factory & factory)
{
  // Kernels
  registerKernel(LevelSetAdvection);
  registerKernel(LevelSetAdvectionSUPG);
  registerKernel(LevelSetTimeDerivativeSUPG);
  registerKernel(LevelSetForcingFunctionSUPG);
  registerKernel(LevelSetOlssonReinitialization);

  // Functions
  registerFunction(LevelSetOlssonBubble);
  registerFunction(LevelSetOlssonVortex);

  // Postprocessors
  registerPostprocessor(LevelSetCFLCondition);
  registerPostprocessor(LevelSetVolume);
  registerPostprocessor(LevelSetOlssonTerminator);

  // Problems
  registerProblem(LevelSetProblem);
  registerProblem(LevelSetReinitializationProblem);

  // MultiApps
  registerMultiApp(LevelSetReinitializationMultiApp);

  // Transfers
  registerTransfer(LevelSetMeshRefinementTransfer);
}

void
LevelSetApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

void
LevelSetApp::registerExecFlags(Factory & factory)
{
  registerExecFlag(LevelSet::EXEC_ADAPT_MESH);
  registerExecFlag(LevelSet::EXEC_COMPUTE_MARKERS);
}

// Dynamic Library Entry Points - DO NOT MODIFY
extern "C" void
LevelSetApp__registerApps()
{
  LevelSetApp::registerApps();
}

extern "C" void
LevelSetApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  LevelSetApp::associateSyntax(syntax, action_factory);
}

extern "C" void
LevelSetApp__registerExecFlags(Factory & factory)
{
  LevelSetApp::registerExecFlags(factory);
}

extern "C" void
LevelSetApp__registerObjects(Factory & factory)
{
  LevelSetApp::registerObjects(factory);
}
