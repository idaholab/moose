#ifndef AUXVARIABLESBLOCK_H
#define AUXVARIABLESBLOCK_H

#include "VariablesBlock.h"

class AuxVariablesBlock;

template<>
InputParameters validParams<AuxVariablesBlock>();

class AuxVariablesBlock: public VariablesBlock
{
public:
  AuxVariablesBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //AUXVARIABLESBLOCK_H
