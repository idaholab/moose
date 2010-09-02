#include "MaterialsBlock.h"

#include "MaterialFactory.h"

template<>
InputParameters validParams<MaterialsBlock>()
{
  return validParams<ParserBlock>();
}

MaterialsBlock::MaterialsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register Materials prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
//  addPrereq("Postprocessors");
}

void
MaterialsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MaterialsBlock Object\n";
#endif

  // Add the Materials to the system
  visitChildren();
}
