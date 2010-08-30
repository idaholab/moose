#ifndef GENERICDAMPERBLOCK_H
#define GENERICDAMPERBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericDamperBlock;

template<>
InputParameters validParams<GenericDamperBlock>();

class GenericDamperBlock: public ParserBlock
{
public:
  GenericDamperBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICDAMPERBLOCK_H
