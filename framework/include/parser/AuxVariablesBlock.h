#ifndef AUXVARIABLESBLOCK_H
#define AUXVARIABLESBLOCK_H

#include "VariablesBlock.h"

class AuxVariablesBlock;

template<>
InputParameters validParams<AuxVariablesBlock>();

class AuxVariablesBlock: public VariablesBlock
{
public:
  AuxVariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //AUXVARIABLESBLOCK_H
