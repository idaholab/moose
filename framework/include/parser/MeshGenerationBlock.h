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
  MeshGenerationBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
  
private:
  bool _executed;
};

#endif //MESHGENERATIONBLOCK_H
