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
#include "NodalAreaAux.h"
#include "NodalAreaAction.h"

void
Elk::Misc::registerObjects()
{
  // Misc
  registerAux(NodalAreaAux);

  registerBoundaryCondition(RobinBC);

  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerKernel(JouleHeating);
  registerKernel(CoefTimeDerivative);
  registerKernel(GaussContForcing);

  registerMaterial(Density);

  registerPostprocessor(InternalVolume);
  registerPostprocessor(SharpInterfaceForcing);

  registerUserObject(NodalArea);
}

void
Elk::Misc::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("NodalAreaAction", "Contact/*");
  registerAction(NodalAreaAction, "add_user_object");
}
