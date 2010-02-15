#include "StabilizersBlock.h"

#include "StabilizerFactory.h"

template<>
InputParameters validParams<StabilizersBlock>()
{
  return validParams<ParserBlock>();
}

StabilizersBlock::StabilizersBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
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
