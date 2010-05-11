#include "AuxKernelsBlock.h"

#include "AuxKernel.h"

template<>
InputParameters validParams<AuxKernelsBlock>()
{
  return validParams<ParserBlock>();
}

AuxKernelsBlock::AuxKernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
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
