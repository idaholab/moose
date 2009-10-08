#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "ParserBlock.h"

class MeshBlock: public ParserBlock
{
public:
  MeshBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

  virtual void execute();
};

  

#endif //MESHBLOCK_H
