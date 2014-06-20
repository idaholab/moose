#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "BodyForceVoid.h"
#include "CoefDiffusion.h"
#include "Convection.h"
#include "Density.h"
#include "InternalVolume.h"
#include "RobinBC.h"
#include "JouleHeating.h"
#include "CoefTimeDerivative.h"
#include "GaussContForcing.h"
#include "SharpInterfaceForcing.h"
#include "RigidBodyModesRZ.h"
#include "RigidBodyModes3D.h"
#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "CInterfacePosition.h"
#include "ThermoDiffusion.h"

template<>
InputParameters validParams<MiscApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

MiscApp::MiscApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  MiscApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MiscApp::associateSyntax(_syntax, _action_factory);
}

MiscApp::~MiscApp()
{
}

void
MiscApp::registerApps()
{
  registerApp(MiscApp);
}

void
MiscApp::registerObjects(Factory & factory)
{
  registerAux(CoupledDirectionalMeshHeightInterpolation);

  registerBoundaryCondition(RobinBC);

  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerKernel(JouleHeating);
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

void
MiscApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
