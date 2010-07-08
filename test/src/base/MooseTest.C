#include "Moose.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"
#include "AuxFactory.h"
#include "InitialConditionFactory.h"

//Local Includes
#include "DiffMKernel.h"

#include "Diff1Material.h"
#include "Diff2Material.h"

namespace MooseTest
{
  void registerObjects()
  {
    KernelFactory::instance()->registerKernel<DiffMKernel>("DiffMKernel");
    
    // Register our new material class so we can use it.
    MaterialFactory::instance()->registerMaterial<Diff1Material>("Diff1Material");
    MaterialFactory::instance()->registerMaterial<Diff2Material>("Diff2Material");
  }
}
