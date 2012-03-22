#include "MiscModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// Misc
#include "BodyForceVoid.h"
#include "CoefDiffusion.h"
#include "Convection.h"
#include "InternalVolume.h"
#include "MaterialRealScaledAux.h"

void
Elk::Misc::registerObjects()
{
  // Misc
  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerAux(MaterialRealScaledAux);
  registerPostprocessor(InternalVolume);
}
