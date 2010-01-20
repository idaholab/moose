#ifndef PERIODICBLOCK_H
#define PERIODICBLOCK_H

#include "ParserBlock.h"

class PeriodicBlock;

template<>
InputParameters validParams<PeriodicBlock>();

class PeriodicBlock: public ParserBlock
{
public:
  PeriodicBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

protected:
  bool _executed;
};


  

#endif //PERIODICBLOCK_H
