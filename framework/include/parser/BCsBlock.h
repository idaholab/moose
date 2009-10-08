#ifndef BCSBLOCK_H
#define BCSBLOCK_H

#include "ParserBlock.h"

class BCsBlock: public ParserBlock
{
public:
  BCsBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

  virtual void execute();
};

  

#endif //BCSBLOCK_H
