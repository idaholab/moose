#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class OutputBlock;

template<>
InputParameters validParams<OutputBlock>();

class OutputBlock: public ParserBlock
{
public:
  OutputBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //OUTPUTBLOCK_H
