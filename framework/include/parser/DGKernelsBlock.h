#ifndef DGKERNELSBLOCK_H
#define DGKERNELSBLOCK_H

#include "ParserBlock.h"

class DGKernelsBlock;

template<>
InputParameters validParams<DGKernelsBlock>();

class DGKernelsBlock: public ParserBlock
{
public:
  DGKernelsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //DGKERNELSBLOCK_H
