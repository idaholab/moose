#include "KernelsBlock.h"

template<>
InputParameters validParams<KernelsBlock>()
{
  return validParams<ParserBlock>();
}


KernelsBlock::KernelsBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
//  addPrereq("AuxVariables");
  addPrereq("Materials");
//  addPrereq("Postprocessors");
}

void
KernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  // Add the kernels to the system
  visitChildren();
}

