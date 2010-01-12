#ifndef ADAPTIVITYBLOCK_H
#define ADAPTIVITYBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class AdaptivityBlock;

template<>
InputParameters validParams<AdaptivityBlock>();

class AdaptivityBlock: public ParserBlock
{
public:
  AdaptivityBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //ADAPTIVITYBLOCK_H
