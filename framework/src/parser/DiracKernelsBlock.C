#include "DiracKernelsBlock.h"
#include "Factory.h"

template<>
InputParameters validParams<DiracKernelsBlock>()
{
  return validParams<ParserBlock>();
}

DiracKernelsBlock::DiracKernelsBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
{
//  // Register execution prereqs
//  addPrereq("Mesh");
//  addPrereq("Variables");
//  addPrereq("Preconditioning");
//  addPrereq("AuxVariables");
//  addPrereq("Materials");
}

void
DiracKernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the DiracKernelsBlock Object\n";
#endif

  // Add the dirac_kernels to the system
  visitChildren();
}  
