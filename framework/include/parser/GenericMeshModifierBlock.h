#ifndef GENERICMESHMODIFIERBLOCK_H
#define GENERICMESHMODIFIERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericMeshModifierBlock;

template<>
InputParameters validParams<GenericMeshModifierBlock>();

class GenericMeshModifierBlock: public ParserBlock
{
public:
  GenericMeshModifierBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

#endif //GENERICMESHMODIFIERBLOCK_H
