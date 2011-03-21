#ifndef KERNELSBLOCK_H_
#define KERNELSBLOCK_H_

#include "ParserBlock.h"


class KernelsBlock: public ParserBlock
{
public:
  KernelsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

template<>
InputParameters validParams<KernelsBlock>();

#endif //KERNELSBLOCK_H_
