#ifndef GENERICKERNELBLOCK_H_
#define GENERICKERNELBLOCK_H_

#include "ParserBlock.h"


class GenericKernelBlock : public ParserBlock
{
public:
  GenericKernelBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

template<>
InputParameters validParams<GenericKernelBlock>();


#endif //GENERICKERNELBLOCK_H
