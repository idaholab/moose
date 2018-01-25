//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "CoefDiffusion.h"
#include "Density.h"
#include "InternalVolume.h"
#include "CoefTimeDerivative.h"
#include "RigidBodyModes3D.h"
#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "ThermoDiffusion.h"

template <>
InputParameters
validParams<MiscApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

MiscApp::MiscApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  MiscApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MiscApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  MiscApp::registerExecFlags(_factory);
}

MiscApp::~MiscApp() {}

// External entry point for dynamic application loading
extern "C" void
MiscApp__registerApps()
{
  MiscApp::registerApps();
}
void
MiscApp::registerApps()
{
  registerApp(MiscApp);
}

// External entry point for dynamic object registration
extern "C" void
MiscApp__registerObjects(Factory & factory)
{
  MiscApp::registerObjects(factory);
}
void
MiscApp::registerObjects(Factory & factory)
{
  registerAux(CoupledDirectionalMeshHeightInterpolation);

  registerKernel(CoefDiffusion);
  registerKernel(CoefTimeDerivative);
  registerKernel(ThermoDiffusion);

  registerMaterial(Density);

  registerUserObject(RigidBodyModes3D);

  registerPostprocessor(InternalVolume);
}

// External entry point for dynamic syntax association
extern "C" void
MiscApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  MiscApp::associateSyntax(syntax, action_factory);
}
void
MiscApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
MiscApp__registerExecFlags(Factory & factory)
{
  MiscApp::registerExecFlags(factory);
}
void
MiscApp::registerExecFlags(Factory & /*factory*/)
{
}
