/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Moose.h"
#include "MooseFactory.h"

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

// kernels
#include "DiffMKernel.h"
#include "MatDiffusion.h"

#include "DGMatDiffusion.h"

#include "Convection.h"
#include "SinContForcing.h"

// DG kernels
#include "DGConvection.h"

// boundary conditions
#include "MTBC.h"
#include "DGMDDBC.h"

#include "DGFunctionConvectionDirichletBC.h"

// materials
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "MTMaterial.h"

namespace MooseTest
{
  void registerObjects()
  {
    registerKernel(DiffMKernel);
    registerKernel(MatDiffusion);
    registerKernel(Convection);
    
    registerKernel(MMSImplicitEuler);

    registerKernel(MMSDiffusion);

    registerKernel(MMSConvection);

    registerKernel(MMSReaction);

    registerKernel(MMSForcing);

    registerKernel(PolyDiffusion);

    registerKernel(PolyConvection);

    registerKernel(PolyReaction);

    registerKernel(PolyForcing);

    registerKernel(SinContForcing);

    registerDGKernel(DGConvection);
    
    // Register our new material class so we can use it.
    registerMaterial(Diff1Material);
    registerMaterial(Diff2Material);
    registerMaterial(MTMaterial);

    //Registering the Boundary Conditions
    registerBC(MMSCoupledDirichletBC);
    registerBC(PolyCoupledDirichletBC);
    registerBC(MTBC);
    registerBC(DGMDDBC);

    registerBC(DGFunctionConvectionDirichletBC);
    
    //Registering our Aux Kernel
    registerAux(MMSConstantAux);

    registerAux(PolyConstantAux);

    registerDGKernel(DGMatDiffusion);
  }
}

