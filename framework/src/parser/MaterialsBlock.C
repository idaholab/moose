#include "MaterialsBlock.h"

#include "MaterialFactory.h"

MaterialsBlock::MaterialsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{}

void
MaterialsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MaterialsBlock Object\n";
#endif

  // Add the Materials to the system
  visitChildren();
}
