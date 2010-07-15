#ifndef FUNCTIONSBLOCK_H
#define FUNCTIONSBLOCK_H

#include "ParserBlock.h"

class FunctionsBlock;

template<>
InputParameters validParams<FunctionsBlock>();

/**
 * All this class does is call visitChildren()
 */
class FunctionsBlock : public ParserBlock
{
public:
  FunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //FUNCTIONSBLOCK_H
