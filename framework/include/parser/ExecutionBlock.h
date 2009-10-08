#ifndef EXECUTIONBLOCK_H
#define EXECUTIONBLOCK_H

#include "ParserBlock.h"

class ExecutionBlock: public ParserBlock
{
public:
  ExecutionBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

  virtual void execute();
};

  

#endif //EXECUTIONBLOCK_H
