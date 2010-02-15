#ifndef GENERICSTABILIZERBLOCK_H
#define GENERICSTABILIZERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericStabilizerBlock;

template<>
InputParameters validParams<GenericStabilizerBlock>();

class GenericStabilizerBlock: public ParserBlock
{
public:
  GenericStabilizerBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICSTABILIZERBLOCK_H
