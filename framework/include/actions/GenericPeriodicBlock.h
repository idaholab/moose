#ifndef GENERICPERIODICBLOCK_H
#define GENERICPERIODICBLOCK_H

#include "ParserBlock.h"

class GenericPeriodicBlock;

template<>
InputParameters validParams<GenericPeriodicBlock>();

class GenericPeriodicBlock : public ParserBlock
{
public:
  GenericPeriodicBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};


  

#endif //GENERICPERIODICBLOCK_H
