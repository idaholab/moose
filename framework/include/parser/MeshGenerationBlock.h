#ifndef MESHGENERATIONBLOCK_H
#define MESHGENERATIONBLOCK_H

#include "ParserBlock.h"

class MeshGenerationBlock: public ParserBlock
{
public:
  MeshGenerationBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file);

  virtual void execute();
  
private:
  bool _executed;
};

#endif //MESHGENERATIONBLOCK_H
