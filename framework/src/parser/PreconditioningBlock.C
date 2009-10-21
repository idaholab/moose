#include "PreconditioningBlock.h"

PreconditioningBlock::PreconditioningBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<std::vector<std::string> >("names");
}

void
PreconditioningBlock::execute() 
{
  // Execute the preconditioning block
  visitChildren();
}  
