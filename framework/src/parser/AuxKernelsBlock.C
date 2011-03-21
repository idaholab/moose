#include "AuxKernelsBlock.h"

template<>
InputParameters validParams<AuxKernelsBlock>()
{
  return validParams<ParserBlock>();
}

AuxKernelsBlock::AuxKernelsBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  addPrereq("Executioner");
#if 0
  // Register AuxKernel prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
#endif
}

void
AuxKernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxKernelsBlock Object\n";
#endif

  // Add the AuxKernels to the system
  visitChildren();
}
