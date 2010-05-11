#ifndef PERIODICBLOCK_H
#define PERIODICBLOCK_H

#include "ParserBlock.h"

class PeriodicBlock;

template<>
InputParameters validParams<PeriodicBlock>();

class PeriodicBlock: public ParserBlock
{
public:
  PeriodicBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

protected:
  bool _executed;
};


  

#endif //PERIODICBLOCK_H
