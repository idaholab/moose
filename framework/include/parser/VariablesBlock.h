#ifndef VARIABLESBLOCK_H
#define VARIABLESBLOCK_H

#include "ParserBlock.h"

class VariablesBlock: public ParserBlock
{
public:
  VariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();
};

  

#endif //VARIABLESBLOCK_H
