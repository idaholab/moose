/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Moose.h"
#include "LevelSetApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

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

template<>
InputParameters validParams<LevelSetApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.addClassDescription("Application containing object necessary to solve the level set equation.");
  return params;
}

LevelSetApp::LevelSetApp(InputParameters parameters) :
    MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  LevelSetApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LevelSetApp::associateSyntax(_syntax, _action_factory);
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
