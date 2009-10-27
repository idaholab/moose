#ifndef PBPBLOCK_H
#define PBPBLOCK_H

#include "ParserBlock.h"

class PBPBlock: public ParserBlock
{
public:
  PBPBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};


  

#endif //PBPBLOCK_H
