#include "MaterialsBlock.h"

#include "MaterialFactory.h"

MaterialsBlock::MaterialsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<std::vector<std::string> >("names");
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
