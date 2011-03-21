#include "BCsBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<BCsBlock>()
{
  return validParams<ParserBlock>();
}


BCsBlock::BCsBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  // Register BCs/AuxBCs prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
//  addPrereq("AuxVariables");
  addPrereq("Kernels");
//  addPrereq("AuxKernels");
}

void
BCsBlock::execute() 
{
#ifdef DEBUG
  if (_parser_handle.pathContains(_name, "BCs"))
    std::cerr << "Inside the BCsBlock Object\n";
  else
    std::cerr << "Inside the BCsBlock (Aux) Object\n";
#endif

  // Add the BCs or AuxBCs to the system
  visitChildren();
}

