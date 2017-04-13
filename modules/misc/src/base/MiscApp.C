/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "BodyForceVoid.h"
#include "CoefDiffusion.h"
#include "Density.h"
#include "InternalVolume.h"
#include "RobinBC.h"
#include "CoefTimeDerivative.h"
#include "GaussContForcing.h"
#include "SharpInterfaceForcing.h"
#include "RigidBodyModesRZ.h"
#include "RigidBodyModes3D.h"
#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "CInterfacePosition.h"
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

  Moose::registerExecFlags();
  MiscApp::registerExecFlags();
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

  registerBoundaryCondition(RobinBC);

  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(CoefTimeDerivative);
  registerKernel(GaussContForcing);
  registerKernel(ThermoDiffusion);

  registerMaterial(Density);

  registerUserObject(RigidBodyModesRZ);
  registerUserObject(RigidBodyModes3D);

  registerPostprocessor(InternalVolume);
  registerPostprocessor(SharpInterfaceForcing);

  registerPostprocessor(CInterfacePosition);
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

void
MiscApp::registerExecFlags()
{
}
