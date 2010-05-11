#ifndef STABILIZERSBLOCK_H
#define STABILIZERSBLOCK_H

#include "ParserBlock.h"

class StabilizersBlock;

template<>
InputParameters validParams<StabilizersBlock>();

class StabilizersBlock: public ParserBlock
{
public:
  StabilizersBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //STABILIZERSBLOCK_H
