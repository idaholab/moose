#ifndef GENERICVARIABLEBLOCK_H
#define GENERICVARIABLEBLOCK_H

#include "ParserBlock.h"

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

  virtual void execute();
};

  

#endif //GENERICVARIABLEBLOCK_H
