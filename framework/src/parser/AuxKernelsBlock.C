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

#include "AuxKernelsBlock.h"

#include "AuxKernel.h"

template<>
InputParameters validParams<AuxKernelsBlock>()
{
  return validParams<ParserBlock>();
}

AuxKernelsBlock::AuxKernelsBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{
  // Register AuxKernel prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
}

void
AuxKernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxKernelsBlock Object\n";
#endif

  /* TODO: Figure out why the AuxKernel::init() breaks here:  It works when in the KernelsBlock
   * module, before the calls to add the regular kernels
   */
//  AuxKernel::init();

  // TODO: Implement GenericAuxKernelBlock
  // Add the AuxKernels to the system
  visitChildren();
}
