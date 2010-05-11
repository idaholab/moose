#ifndef MESHGENERATIONBLOCK_H
#define MESHGENERATIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class MeshGenerationBlock;

template<>
InputParameters validParams<MeshGenerationBlock>();

class MeshGenerationBlock: public ParserBlock
{
public:
  MeshGenerationBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
  
private:
  bool _executed;
};

#endif //MESHGENERATIONBLOCK_H
