#include "PreconditioningBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

PreconditioningBlock::PreconditioningBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{}

void
PreconditioningBlock::execute() 
{
  // Execute the preconditioning block
  visitChildren();
}  
