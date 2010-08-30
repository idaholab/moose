#include "DampersBlock.h"

#include "DamperFactory.h"

template<>
InputParameters validParams<DampersBlock>()
{
  return validParams<ParserBlock>();
}

DampersBlock::DampersBlock(std::string name, MooseSystem & moose_system, InputParameters params)
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
DampersBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the DampersBlock Object\n";
#endif

  // Add the dampers to the system
  visitChildren();
}  

  
