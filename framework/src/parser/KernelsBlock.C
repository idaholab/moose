#include "KernelsBlock.h"

#include "KernelFactory.h"
#include "AuxKernel.h"
#include "TransientBlock.h"

template<>
InputParameters validParams<KernelsBlock>()
{
  return validParams<ParserBlock>();
}

KernelsBlock::KernelsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
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
