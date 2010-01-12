#ifndef GENERICBCBLOCK_H
#define GENERICBCBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericBCBlock;

template<>
InputParameters validParams<GenericBCBlock>();

class GenericBCBlock: public ParserBlock
{
public:
  GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICBCBLOCK_H
