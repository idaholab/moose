#ifndef DAMPERSBLOCK_H
#define DAMPERSBLOCK_H

#include "ParserBlock.h"

class DampersBlock;

template<>
InputParameters validParams<DampersBlock>();

class DampersBlock: public ParserBlock
{
public:
  DampersBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //DAMPERSBLOCK_H
