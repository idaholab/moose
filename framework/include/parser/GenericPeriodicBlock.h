#ifndef GENERICPERIODICBLOCK_H
#define GENERICPERIODICBLOCK_H

#include "ParserBlock.h"

class GenericPeriodicBlock;

template<>
InputParameters validParams<GenericPeriodicBlock>();

class GenericPeriodicBlock: public ParserBlock
{
public:
  GenericPeriodicBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};


  

#endif //GENERICPERIODICBLOCK_H
