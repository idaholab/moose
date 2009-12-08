#ifndef BCSBLOCK_H
#define BCSBLOCK_H

#include "ParserBlock.h"

class BCsBlock;

template<>
InputParameters validParams<BCsBlock>()
{
  return validParams<ParserBlock>();
}

class BCsBlock: public ParserBlock
{
public:
  BCsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //BCSBLOCK_H
