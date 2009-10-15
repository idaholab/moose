#ifndef AUXVARIABLESBLOCK_H
#define AUXVARIABLESBLOCK_H

#include "VariablesBlock.h"

class AuxVariablesBlock: public VariablesBlock
{
public:
  AuxVariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();
};

  

#endif //AUXVARIABLESBLOCK_H
