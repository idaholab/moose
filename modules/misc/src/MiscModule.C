#include "MiscModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// Misc
#include "BodyForceVoid.h"
#include "CoefDiffusion.h"
#include "Convection.h"
#include "InternalVolume.h"
#include "MaterialRealScaledAux.h"
#include "RobinBC.h"
#include "JouleHeating.h"
#include "CoefTimeDerivative.h"

void
Elk::Misc::registerObjects()
{
  // Misc
  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerKernel(JouleHeating);
  registerKernel(CoefTimeDerivative);
  registerAux(MaterialRealScaledAux);
  registerPostprocessor(InternalVolume);
  registerBoundaryCondition(RobinBC);
}
