#include "Moose.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"
#include "AuxFactory.h"
#include "InitialConditionFactory.h"

//Our Example Includes
#include "MMSImplicitEuler.h" //including Implicit Euler
#include "MMSCoupledDirichletBC.h" //including our Boundary Conditions
#include "MMSDiffusion.h" //including our Diffusion Kernel
#include "MMSConvection.h" //including our Forcing Kernel
#include "MMSReaction.h" //including our Convection Kernel
#include "MMSForcing.h" //including our Reaction Kernel
#include "MMSConstantAux.h" //including our Aux Kerne

//Local Includes
#include "DiffMKernel.h"

#include "Diff1Material.h"
#include "Diff2Material.h"

namespace MooseTest
{
  void registerObjects()
  {
    KernelFactory::instance()->registerKernel<DiffMKernel>("DiffMKernel");

    KernelFactory::instance()->registerKernel<MMSImplicitEuler>("MMSImplicitEuler");

    KernelFactory::instance()->registerKernel<MMSDiffusion>("MMSDiffusion");

    KernelFactory::instance()->registerKernel<MMSConvection>("MMSConvection");

    KernelFactory::instance()->registerKernel<MMSReaction>("MMSReaction");

    KernelFactory::instance()->registerKernel<MMSForcing>("MMSForcing");  

    // Register our new material class so we can use it.
    MaterialFactory::instance()->registerMaterial<Diff1Material>("Diff1Material");
    MaterialFactory::instance()->registerMaterial<Diff2Material>("Diff2Material");

    //Registering the Boundary Conditions
  BCFactory::instance()->registerBC<MMSCoupledDirichletBC>("MMSCoupledDirichletBC");

//Registering our Aux Kernel
  AuxFactory::instance()->registerAux<MMSConstantAux>("MMSConstantAux");
  }
}
