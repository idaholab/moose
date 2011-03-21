#ifndef MESHGENERATIONBLOCK_H_
#define MESHGENERATIONBLOCK_H_

#include "ParserBlock.h"


class MeshGenerationBlock: public ParserBlock
{
public:
  MeshGenerationBlock(const std::string & name, InputParameters params);

  virtual void execute();
  
private:
  bool _executed;
};

template<>
InputParameters validParams<MeshGenerationBlock>();

#endif //MESHGENERATIONBLOCK_H
