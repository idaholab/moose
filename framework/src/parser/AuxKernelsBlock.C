#include "AuxKernelsBlock.h"

#include "AuxKernel.h"

AuxKernelsBlock::AuxKernelsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle)
{}

void
AuxKernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxKernelsBlock Object\n";
#endif

  /* TODO: Figure out why the AuxKernel::init() breaks here:  It works when in the KernelsBlock
   * module, before the calls to add the regular kernels
   */
  //AuxKernel::init();

  // TODO: Implement GenericAuxKernelBlock
  // Add the AuxKernels to the system
  visitChildren();
}
