#ifndef GENERICDGKERNELBLOCK_H
#define GENERICDGKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericDGKernelBlock;

template<>
InputParameters validParams<GenericDGKernelBlock>();

class GenericDGKernelBlock: public ParserBlock
{
public:
  GenericDGKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICDGKERNELBLOCK_H
