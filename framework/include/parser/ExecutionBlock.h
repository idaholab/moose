#ifndef EXECUTIONBLOCK_H
#define EXECUTIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class ExecutionBlock;

template<>
InputParameters validParams<ExecutionBlock>();

class ExecutionBlock: public ParserBlock
{
public:
  ExecutionBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //EXECUTIONBLOCK_H
