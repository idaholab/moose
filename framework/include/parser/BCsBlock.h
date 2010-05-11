#ifndef BCSBLOCK_H
#define BCSBLOCK_H

#include "ParserBlock.h"

class BCsBlock;

template<>
InputParameters validParams<BCsBlock>();

class BCsBlock: public ParserBlock
{
public:
  BCsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //BCSBLOCK_H
