#ifndef BCBLOCK_H
#define BCBLOCK_H

#include "ParserBlock.h"

class BCBlock: public ParserBlock
{
public:
  BCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};

  

#endif //BCBLOCK_H
