#ifndef MATERIALSBLOCK_H
#define MATERIALSBLOCK_H

#include "ParserBlock.h"

class MaterialsBlock;

template<>
InputParameters validParams<MaterialsBlock>()
{
  return validParams<ParserBlock>();
}

class MaterialsBlock: public ParserBlock
{
public:
  MaterialsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //MATERIALSBLOCK_H
