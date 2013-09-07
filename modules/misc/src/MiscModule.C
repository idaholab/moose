#include "MiscModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// Misc
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
#include "NodalArea.h"
#include "NodalAreaAction.h"
#include "NodalAreaVarAction.h"
#include "RigidBodyModesRZ.h"
#include "RigidBodyModes3D.h"
#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "CInterfacePosition.h"

void
Elk::Misc::registerObjects(Factory & factory)
{
  // Misc
  registerAux(CoupledDirectionalMeshHeightInterpolation);

  registerBoundaryCondition(RobinBC);

  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerKernel(JouleHeating);
  registerKernel(CoefTimeDerivative);
  registerKernel(GaussContForcing);

  registerMaterial(Density);

  registerUserObject(RigidBodyModesRZ);
  registerUserObject(RigidBodyModes3D);

  registerPostprocessor(InternalVolume);
  registerPostprocessor(SharpInterfaceForcing);

  registerUserObject(NodalArea);
  registerPostprocessor(CInterfacePosition);
}

void
Elk::Misc::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("NodalAreaAction", "Contact/*");
  syntax.registerActionSyntax("NodalAreaVarAction", "Contact/*");
  registerAction(NodalAreaAction, "add_user_object");
  registerAction(NodalAreaVarAction, "add_aux_variable");
}
