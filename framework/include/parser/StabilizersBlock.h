#ifndef STABILIZERSBLOCK_H_
#define STABILIZERSBLOCK_H_

#include "ParserBlock.h"

class StabilizersBlock;

template<>
InputParameters validParams<StabilizersBlock>();

class StabilizersBlock: public ParserBlock
{
public:
  StabilizersBlock(const std::string & name, InputParameters params);

  virtual void execute();
};


  

#endif //STABILIZERSBLOCK_H
