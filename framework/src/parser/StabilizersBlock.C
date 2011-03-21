#include "StabilizersBlock.h"
#include "Factory.h"

template<>
InputParameters validParams<StabilizersBlock>()
{
  return validParams<ParserBlock>();
}

StabilizersBlock::StabilizersBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
{
//  // Register execution prereqs
//  addPrereq("Mesh");
//  addPrereq("Variables");
//  addPrereq("AuxVariables");
//  addPrereq("Kernels");
}

void
StabilizersBlock::execute() 
{
  // Add the stabilizers to the system
  visitChildren();
}  
