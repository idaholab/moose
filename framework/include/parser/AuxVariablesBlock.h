#ifndef AUXVARIABLESBLOCK_H_
#define AUXVARIABLESBLOCK_H_

#include "VariablesBlock.h"

class AuxVariablesBlock;

template<>
InputParameters validParams<AuxVariablesBlock>();

class AuxVariablesBlock: public VariablesBlock
{
public:
  AuxVariablesBlock(const std::string & name, InputParameters params);
  
  virtual void execute();
};


#endif //AUXVARIABLESBLOCK_H_
