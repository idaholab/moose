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
  ExecutionBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //EXECUTIONBLOCK_H
