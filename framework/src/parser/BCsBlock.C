#include "BCsBlock.h"

#include "BCFactory.h"

BCsBlock::BCsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
}

void
BCsBlock::execute() 
{
#ifdef DEBUG
  if (_reg_id == "BCs")
    std::cerr << "Inside the BCsBlock Object\n";
  else
    std::cerr << "Inside the BCsBlock (Aux) Object\n";
#endif

  if (_reg_id == "BCs")
    BoundaryCondition::init();
  
  // Add the BCs or AuxBCs to the system
  visitChildren();
}
