#include "MaterialsBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<MaterialsBlock>()
{
  return validParams<ParserBlock>();
}


MaterialsBlock::MaterialsBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params)
{
  if (!_parser_handle._loose)
    addPrereq("Variables");
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

