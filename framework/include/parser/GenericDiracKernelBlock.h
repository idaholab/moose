#ifndef GENERICDIRACKERNELBLOCK_H_
#define GENERICDIRACKERNELBLOCK_H_

#include "ParserBlock.h"

//Forward Declarations
class GenericDiracKernelBlock;

template<>
InputParameters validParams<GenericDiracKernelBlock>();

class GenericDiracKernelBlock: public ParserBlock
{
public:
  GenericDiracKernelBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

  

#endif //GENERICDIRACKERNELBLOCK_H_
