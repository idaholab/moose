#include "MaterialsBlock.h"
#include "Factory.h"

template<>
InputParameters validParams<MaterialsBlock>()
{
  return validParams<ParserBlock>();
}


MaterialsBlock::MaterialsBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  addPrereq("Executioner");
#if 0
  // Register Materials prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
//  addPrereq("AuxVariables");
  addPrereq("Executioner");
#endif
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

