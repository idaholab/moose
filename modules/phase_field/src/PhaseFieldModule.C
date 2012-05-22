#include "PhaseFieldModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// phase_field
#include "MatDiffusion.h"
#include "ACInterface.h"
#include "CHMath.h"
#include "CHInterface.h"
#include "SplitCHWRes.h"
#include "SplitCHMath.h"
#include "CoupledImplicitEuler.h"
#include "CrossIC.h"
#include "SmoothCircleIC.h"
#include "RndSmoothCircleIC.h"
#include "MultiSmoothCircleIC.h"
#include "LatticeSmoothCircleIC.h"
#include "RndBoundingBoxIC.h"
#include "PFMobility.h"
#include "NodalFloodCount.h"

void
Elk::PhaseField::registerObjects()
{
  // phase_field
  registerKernel(MatDiffusion);
  registerKernel(ACInterface);
  registerKernel(CHMath);
  registerKernel(CHInterface);
  registerKernel(SplitCHWRes);
  registerKernel(SplitCHMath);
  registerKernel(CoupledImplicitEuler);
  registerInitialCondition(CrossIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(MultiSmoothCircleIC);
  registerInitialCondition(LatticeSmoothCircleIC);
  registerInitialCondition(RndBoundingBoxIC);
  registerMaterial(PFMobility);
  registerPostprocessor(NodalFloodCount);
}
