#ifndef MATERIALSBLOCK_H
#define MATERIALSBLOCK_H

#include "ParserBlock.h"

class MaterialsBlock;

template<>
InputParameters validParams<MaterialsBlock>();

class MaterialsBlock: public ParserBlock
{
public:
  MaterialsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //MATERIALSBLOCK_H
