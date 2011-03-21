#ifndef GENERICMATERIALBLOCK_H_
#define GENERICMATERIALBLOCK_H_

#include "ParserBlock.h"

class GenericMaterialBlock: public ParserBlock
{
public:
  GenericMaterialBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

template<>
InputParameters validParams<GenericMaterialBlock>();

#endif //GENERICMATERIALBLOCK_H
