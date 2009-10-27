#include "PreconditioningBlock.h"

PreconditioningBlock::PreconditioningBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle)
{}

void
PreconditioningBlock::execute() 
{
  // Execute the preconditioning block
  visitChildren();
}  
