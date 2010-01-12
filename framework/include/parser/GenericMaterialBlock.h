#ifndef GENERICMATERIALBLOCK_H
#define GENERICMATERIALBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericMaterialBlock;

template<>
InputParameters validParams<GenericMaterialBlock>();

class GenericMaterialBlock: public ParserBlock
{
public:
  GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICMATERIALBLOCK_H
