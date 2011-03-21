#ifndef BCSBLOCK_H_
#define BCSBLOCK_H_

#include "ParserBlock.h"

class BCsBlock: public ParserBlock
{
public:
  BCsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};


template<>
InputParameters validParams<BCsBlock>();

#endif //BCSBLOCK_H_
