#ifndef ADAPTIVITYBLOCK_H
#define ADAPTIVITYBLOCK_H

#include "ParserBlock.h"

class AdaptivityBlock: public ParserBlock
{
public:
  AdaptivityBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};

  

#endif //ADAPTIVITYBLOCK_H
