#ifndef AUXKERNELSBLOCK_H
#define AUXKERNELSBLOCK_H

#include "ParserBlock.h"

class AuxKernelsBlock;

template<>
InputParameters validParams<AuxKernelsBlock>();

class AuxKernelsBlock: public ParserBlock
{
public:
  AuxKernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //AUXKERNELSBLOCK_H
