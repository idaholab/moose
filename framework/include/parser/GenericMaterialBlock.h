#ifndef GENERICMATERIALBLOCK_H
#define GENERICMATERIALBLOCK_H

#include "ParserBlock.h"

class GenericMaterialBlock: public ParserBlock
{
public:
  GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICMATERIALBLOCK_H
