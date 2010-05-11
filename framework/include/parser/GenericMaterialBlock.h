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
  GenericMaterialBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICMATERIALBLOCK_H
