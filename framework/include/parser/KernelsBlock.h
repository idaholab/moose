#ifndef KERNELSBLOCK_H
#define KERNELSBLOCK_H

#include "ParserBlock.h"

class KernelsBlock: public ParserBlock
{
public:
  KernelsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();
};


  

#endif //KERNELSBLOCK_H
