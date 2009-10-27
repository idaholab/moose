#ifndef AUXKERNELSBLOCK_H
#define AUXKERNELSBLOCK_H

#include "ParserBlock.h"

class AuxKernelsBlock: public ParserBlock
{
public:
  AuxKernelsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};

  

#endif //AUXKERNELSBLOCK_H
