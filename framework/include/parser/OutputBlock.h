#ifndef OUTPUTBLOCK_H_
#define OUTPUTBLOCK_H_

#include "ParserBlock.h"

class OutputBlock : public ParserBlock
{
public:
  OutputBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

template<>
InputParameters validParams<OutputBlock>();

#endif //OUTPUTBLOCK_H
