#include "KernelFactory.h"
#include "CoefDiffusion.h"

#include "BCFactory.h"

#include "AuxFactory.h"

#include "MaterialFactory.h"

#include "Elk.h"

void
Elk::registerObjects()
{
  KernelFactory::instance()->registerKernel<CoefDiffusion>("CoefDiffusion");
}
