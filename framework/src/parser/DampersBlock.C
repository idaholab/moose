#include "DampersBlock.h"

#include "Factory.h"

template<>
InputParameters validParams<DampersBlock>()
{
  return validParams<ParserBlock>();
}

DampersBlock::DampersBlock(const std::string & name, InputParameters params) :
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
DampersBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the DampersBlock Object\n";
#endif

  // Add the dampers to the system
  visitChildren();
}  

  
