#ifndef BCBLOCK_H
#define BCBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
//class InputParameters;
class Parser;
class BCBlock;

template<>
InputParameters validParams<BCBlock>()
{
  return validParams<ParserBlock>();
}

class BCBlock: public ParserBlock
{
public:
  BCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //BCBLOCK_H
