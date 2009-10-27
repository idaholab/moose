#ifndef MATERIALSBLOCK_H
#define MATERIALSBLOCK_H

#include "ParserBlock.h"

class MaterialsBlock: public ParserBlock
{
public:
  MaterialsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();
};

  

#endif //MATERIALSBLOCK_H
