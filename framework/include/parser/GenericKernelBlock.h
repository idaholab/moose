#ifndef GENERICKERNELBLOCK_H
#define GENERICKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericKernelBlock;

template<>
InputParameters validParams<GenericKernelBlock>();

class GenericKernelBlock: public ParserBlock
{
public:
  GenericKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICKERNELBLOCK_H
