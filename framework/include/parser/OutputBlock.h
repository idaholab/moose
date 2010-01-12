#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class OutputBlock;

template<>
InputParameters validParams<OutputBlock>();

class OutputBlock: public ParserBlock
{
public:
  OutputBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

#endif //OUTPUTBLOCK_H
