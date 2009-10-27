#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include "ParserBlock.h"

class OutputBlock: public ParserBlock
{
public:
  OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};

  

#endif //OUTPUTBLOCK_H
