#ifndef PRECONDITIONINGBLOCK_H
#define PRECONDITIONINGBLOCK_H

#include "ParserBlock.h"

class PreconditioningBlock;

template<>
InputParameters validParams<PreconditioningBlock>();

class PreconditioningBlock: public ParserBlock
{
public:
  PreconditioningBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //PRECONDITIONINGBLOCK_H
