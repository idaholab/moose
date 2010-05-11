#ifndef GENERICPERIODICBLOCK_H
#define GENERICPERIODICBLOCK_H

#include "ParserBlock.h"

class GenericPeriodicBlock;

template<>
InputParameters validParams<GenericPeriodicBlock>();

class GenericPeriodicBlock: public ParserBlock
{
public:
  GenericPeriodicBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //GENERICPERIODICBLOCK_H
