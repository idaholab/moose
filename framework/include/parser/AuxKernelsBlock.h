#ifndef AUXKERNELSBLOCK_H_
#define AUXKERNELSBLOCK_H_

#include "ParserBlock.h"

class AuxKernelsBlock;

template<>
InputParameters validParams<AuxKernelsBlock>();

class AuxKernelsBlock: public ParserBlock
{
public:
  AuxKernelsBlock(const std::string & name, InputParameters params);
  
  virtual void execute();
};

  

#endif //AUXKERNELSBLOCK_H_
