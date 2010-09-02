/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "KernelsBlock.h"

template<>
InputParameters validParams<KernelsBlock>()
{
  return validParams<ParserBlock>();
}

KernelsBlock::KernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Materials");
  addPrereq("Postprocessors");
}

void
KernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  // See if there is a transient block and setup time params before calling Kernel::init()

// DEPRECATED
//  TransientBlock * t_block = dynamic_cast<TransientBlock *>(locateBlock("Execution/Transient"));
//  if (t_block != NULL) 
//    t_block->setOutOfOrderTransientParams(_moose_system.getEquationSystems()->parameters);
// DEPRECATED

  // Add the kernels to the system
  visitChildren();
}  

  
