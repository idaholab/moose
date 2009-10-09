#ifndef GENERICVARIABLEBLOCK_H
#define GENERICVARIABLEBLOCK_H

#include "ParserBlock.h"

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();

  bool restartRequired() const;
};

  

#endif //GENERICVARIABLEBLOCK_H
