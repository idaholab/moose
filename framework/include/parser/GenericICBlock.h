#ifndef GENERICICBLOCK_H
#define GENERICICBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericICBlock;

template<>
InputParameters validParams<GenericICBlock>();

class GenericICBlock: public ParserBlock
{
public:
  GenericICBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICICBLOCK_H
