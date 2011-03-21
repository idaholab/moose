#ifndef MATERIALSBLOCK_H_
#define MATERIALSBLOCK_H_

#include "ParserBlock.h"


class MaterialsBlock : public ParserBlock
{
public:
  MaterialsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

template<>
InputParameters validParams<MaterialsBlock>();

#endif //MATERIALSBLOCK_H
