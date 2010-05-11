#include "StabilizersBlock.h"

#include "StabilizerFactory.h"

template<>
InputParameters validParams<StabilizersBlock>()
{
  return validParams<ParserBlock>();
}

StabilizersBlock::StabilizersBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
}

void
StabilizersBlock::execute() 
{
  // Add the stabilizers to the system
  visitChildren();
}  
