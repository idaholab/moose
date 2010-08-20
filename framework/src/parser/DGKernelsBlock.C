#include "DGKernelsBlock.h"

template<>
InputParameters validParams<DGKernelsBlock>()
{
  return validParams<ParserBlock>();
}

DGKernelsBlock::DGKernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Materials");
}

void
DGKernelsBlock::execute()
{
#ifdef DEBUG
  std::cerr << "Inside the DGKernelsBlock Object\n";
#endif

  // Add the kernels to the system
  visitChildren();
}  
