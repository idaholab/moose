#ifndef DIRACKERNELSBLOCK_H_
#define DIRACKERNELSBLOCK_H_

#include "ParserBlock.h"

class DiracKernelsBlock;

template<>
InputParameters validParams<DiracKernelsBlock>();

class DiracKernelsBlock: public ParserBlock
{
public:
  DiracKernelsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

#endif //DIRACKERNELSBLOCK_H_
