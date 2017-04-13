/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// BCs
#include "AEFVBC.h"

// Materials
#include "AEFVMaterial.h"

// DG kernels
#include "AEFVKernel.h"

// User objects
#include "AEFVSlopeReconstructionOneD.h"
#include "AEFVSlopeLimitingOneD.h"
#include "AEFVUpwindInternalSideFlux.h"
#include "AEFVFreeOutflowBoundaryFlux.h"

template <>
InputParameters
validParams<RdgApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

RdgApp::RdgApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RdgApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RdgApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags();
  RdgApp::registerExecFlags();
}

RdgApp::~RdgApp() {}

// External entry point for dynamic application loading
extern "C" void
RdgApp__registerApps()
{
  RdgApp::registerApps();
}
void
RdgApp::registerApps()
{
  registerApp(RdgApp);
}

// External entry point for dynamic object registration
extern "C" void
RdgApp__registerObjects(Factory & factory)
{
  RdgApp::registerObjects(factory);
}
void
RdgApp::registerObjects(Factory & factory)
{
  // DG kernels
  registerDGKernel(AEFVKernel);

  // BCs
  registerBoundaryCondition(AEFVBC);

  // Materials
  registerMaterial(AEFVMaterial);

  // User objects
  registerUserObject(AEFVSlopeReconstructionOneD);
  registerUserObject(AEFVSlopeLimitingOneD);
  registerUserObject(AEFVUpwindInternalSideFlux);
  registerUserObject(AEFVFreeOutflowBoundaryFlux);
}

// External entry point for dynamic syntax association
extern "C" void
RdgApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RdgApp::associateSyntax(syntax, action_factory);
}
void
RdgApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

void
RdgApp::registerExecFlags()
{
}
