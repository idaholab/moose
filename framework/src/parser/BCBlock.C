#include "BCBlock.h"

BCBlock::BCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle)
{}

void
BCBlock::execute() 
{
  std::cout << "***BCBlock Class***\n";
  ParserBlock::execute();
}
