#ifndef GENERICAUXKERNELBLOCK_H
#define GENERICAUXKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericAuxKernelBlock;

template<>
InputParameters validParams<GenericAuxKernelBlock>();

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H
