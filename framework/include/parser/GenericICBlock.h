#ifndef GENERICICBLOCK_H
#define GENERICICBLOCK_H

#include "ParserBlock.h"

class GenericICBlock: public ParserBlock
{
public:
  GenericICBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICICBLOCK_H
