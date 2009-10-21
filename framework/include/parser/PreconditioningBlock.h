#ifndef PRECONDITIONINGBLOCK_H
#define PRECONDITIONINGBLOCK_H

#include "ParserBlock.h"

class PreconditioningBlock: public ParserBlock
{
public:
  PreconditioningBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();
};


  

#endif //PRECONDITIONINGBLOCK_H
