#ifndef FUNCTIONSBLOCK_H_
#define FUNCTIONSBLOCK_H_

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
  FunctionsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

#endif //FUNCTIONSBLOCK_H_
