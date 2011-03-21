#include "PostprocessorsBlock.h"

template<>
InputParameters validParams<PostprocessorsBlock>()
{
  return validParams<ParserBlock>();
}

PostprocessorsBlock::PostprocessorsBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
{
  addPrereq("Executioner");
#if 0
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Materials");
#endif
}

void
PostprocessorsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the PostprocessorsBlock Object\n";
#endif

  // Add the postprocessors to the system
  visitChildren();
}  

  
