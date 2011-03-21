#ifndef GENERICAUXKERNELBLOCK_H_
#define GENERICAUXKERNELBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericAuxKernelBlock;

template<>
InputParameters validParams<GenericAuxKernelBlock>();

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H_
