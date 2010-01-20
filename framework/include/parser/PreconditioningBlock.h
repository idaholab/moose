#ifndef PRECONDITIONINGBLOCK_H
#define PRECONDITIONINGBLOCK_H

#include "ParserBlock.h"

class PreconditioningBlock;

template<>
InputParameters validParams<PreconditioningBlock>();

class PreconditioningBlock: public ParserBlock
{
public:
  PreconditioningBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

#endif //PRECONDITIONINGBLOCK_H
