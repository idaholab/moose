#include "KernelsBlock.h"

#include "KernelFactory.h"
#include "AuxKernel.h"
#include "TransientBlock.h"

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
  addPrereq("AuxVariables");
}

void
KernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  // See if there is a transient block and setup time params before calling Kernel::init()
  TransientBlock * t_block = dynamic_cast<TransientBlock *>(locateBlock("Execution/Transient"));
  if (t_block != NULL) 
    t_block->setOutOfOrderTransientParams(_moose_system.getEquationSystems()->parameters);

  // Add the kernels to the system
  visitChildren();
}  
