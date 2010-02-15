#ifndef STABILIZERSBLOCK_H
#define STABILIZERSBLOCK_H

#include "ParserBlock.h"

class StabilizersBlock;

template<>
InputParameters validParams<StabilizersBlock>();

class StabilizersBlock: public ParserBlock
{
public:
  StabilizersBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};


  

#endif //STABILIZERSBLOCK_H
