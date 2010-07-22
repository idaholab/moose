#include "Moose.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"
#include "AuxFactory.h"
#include "InitialConditionFactory.h"

//Our Example Includes
#include "MMSImplicitEuler.h" //including our sine Implicit Euler Kernel
#include "MMSCoupledDirichletBC.h" //including our sine Boundary Conditions
#include "MMSDiffusion.h" //including our sine Diffusion Kernel
#include "MMSConvection.h" //including our sine Forcing Kernel
#include "MMSReaction.h" //including our sine Convection Kernel
#include "MMSForcing.h" //including our sine Reaction Kernel
#include "MMSConstantAux.h" //including our sine Aux Kernel
#include "PolyCoupledDirichletBC.h" //including our polynomial Boundary Conditions
#include "PolyDiffusion.h" //including our polynomial Diffusion Kernel
#include "PolyConvection.h" //including our polynomial Forcing Kernel
#include "PolyReaction.h" //including our polynomial Convection Kernel
#include "PolyForcing.h" //including our polynomial Reaction Kernel
#include "PolyConstantAux.h" //including our polynomial Aux Kernel
#include "PenetrationAux.h"
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

    KernelFactory::instance()->registerKernel<PolyDiffusion>("PolyDiffusion");

    KernelFactory::instance()->registerKernel<PolyConvection>("PolyConvection");

    KernelFactory::instance()->registerKernel<PolyReaction>("PolyReaction");

    KernelFactory::instance()->registerKernel<PolyForcing>("PolyForcing");

    // Register our new material class so we can use it.
    MaterialFactory::instance()->registerMaterial<Diff1Material>("Diff1Material");
    MaterialFactory::instance()->registerMaterial<Diff2Material>("Diff2Material");

    //Registering the Boundary Conditions
    BCFactory::instance()->registerBC<MMSCoupledDirichletBC>("MMSCoupledDirichletBC");
    BCFactory::instance()->registerBC<PolyCoupledDirichletBC>("PolyCoupledDirichletBC");
    //Registering our Aux Kernel
    AuxFactory::instance()->registerAux<MMSConstantAux>("MMSConstantAux");

    AuxFactory::instance()->registerAux<PolyConstantAux>("PolyConstantAux");

    AuxFactory::instance()->registerAux<PenetrationAux>("PenetrationAux");
  }
}

