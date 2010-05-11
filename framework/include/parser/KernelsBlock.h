#ifndef KERNELSBLOCK_H
#define KERNELSBLOCK_H

#include "ParserBlock.h"

class KernelsBlock;

template<>
InputParameters validParams<KernelsBlock>();

class KernelsBlock: public ParserBlock
{
public:
  KernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //KERNELSBLOCK_H
