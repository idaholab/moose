#ifndef GENERICFUNCTIONSBLOCK_H
#define GENERICFUNCTIONSBLOCK_H

#include "InputParameters.h"
#include "ParserBlock.h"

//Forward Declarations
class GenericFunctionsBlock;

template<>
InputParameters validParams<GenericFunctionsBlock>();

class GenericFunctionsBlock: public ParserBlock
{
public:
  GenericFunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

#endif //GENERICFUNCTIONSBLOCK_H
